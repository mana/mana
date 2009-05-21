/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "net/ea/tradehandler.h"

#include "net/ea/protocol.h"

#include "net/inventoryhandler.h"
#include "net/messagein.h"
#include "net/messageout.h"

#include "inventory.h"
#include "item.h"
#include "localplayer.h"
#include "playerrelations.h"

#include "gui/confirmdialog.h"
#include "gui/trade.h"

#include "gui/widgets/chattab.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

std::string tradePartnerName;

/**
 * Listener for request trade dialogs
 */
namespace {
    struct RequestTradeListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            Net::getTradeHandler()->respond(event.getId() == "yes");
        }
    } listener;
}

Net::TradeHandler *tradeHandler;

namespace EAthena {

TradeHandler::TradeHandler()
{
    static const Uint16 _messages[] = {
        SMSG_TRADE_REQUEST,
        SMSG_TRADE_RESPONSE,
        SMSG_TRADE_ITEM_ADD,
        SMSG_TRADE_ITEM_ADD_RESPONSE,
        SMSG_TRADE_OK,
        SMSG_TRADE_CANCEL,
        SMSG_TRADE_COMPLETE,
        0
    };
    handledMessages = _messages;
    tradeHandler = this;
}


void TradeHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_TRADE_REQUEST:
                // If a trade window or request window is already open, send a
                // trade cancel to any other trade request.
                //
                // Note that it would be nice if the server would prevent this
                // situation, and that the requesting player would get a
                // special message about the player being occupied.
                tradePartnerName = msg.readString(24);

                if (player_relations.hasPermission(tradePartnerName,
                                                   PlayerRelation::TRADE))
                {
                    if (!player_node->tradeRequestOk())
                    {
                        Net::getTradeHandler()->respond(false);
                        break;
                    }

                    player_node->setTrading(true);
                    ConfirmDialog *dlg;
                    dlg = new ConfirmDialog(_("Request for Trade"),
                            strprintf(_("%s wants to trade with you, do you "
                                    "accept?"), tradePartnerName.c_str()));
                    dlg->addActionListener(&listener);
                }
                else
                {
                    Net::getTradeHandler()->respond(false);
                    break;
                }
            break;

        case SMSG_TRADE_RESPONSE:
            switch (msg.readInt8())
            {
                case 0: // Too far away
                    localChatTab->chatLog(_("Trading isn't possible. Trade "
                            "partner is too far away."), BY_SERVER);
                    break;
                case 1: // Character doesn't exist
                    localChatTab->chatLog(_("Trading isn't possible. Character "
                            "doesn't exist."), BY_SERVER);
                    break;
                case 2: // Invite request check failed...
                    localChatTab->chatLog(_("Trade cancelled due to an unknown "
                            "reason."), BY_SERVER);
                    break;
                case 3: // Trade accepted
                    tradeWindow->reset();
                    tradeWindow->setCaption(strprintf(_("Trade: You and %s"),
                            tradePartnerName.c_str()));
                    tradeWindow->setVisible(true);
                    break;
                case 4: // Trade cancelled
                    if (player_relations.hasPermission(tradePartnerName,
                                                       PlayerRelation::SPEECH_LOG))
                        localChatTab->chatLog(strprintf(_("Trade with %s "
                                "cancelled."), tradePartnerName.c_str()),
                                BY_SERVER);
                    // otherwise ignore silently

                    tradeWindow->setVisible(false);
                    player_node->setTrading(false);
                    break;
                default: // Shouldn't happen as well, but to be sure
                    localChatTab->chatLog(_("Unhandled trade cancel packet."),
                            BY_SERVER);
                    break;
            }
            break;

        case SMSG_TRADE_ITEM_ADD:
            {
                int amount = msg.readInt32();
                int type = msg.readInt16();
                msg.readInt8();  // identified flag
                msg.readInt8();  // attribute
                msg.readInt8();  // refine
                msg.skip(8);     // card (4 shorts)

                // TODO: handle also identified, etc
                if (type == 0) {
                    tradeWindow->setMoney(amount);
                } else {
                    tradeWindow->addItem(type, false, amount, false);
                }
            }
            break;

        case SMSG_TRADE_ITEM_ADD_RESPONSE:
            // Trade: New Item add response (was 0x00ea, now 01b1)
            {
                const int index = msg.readInt16() - INVENTORY_OFFSET;
                Item *item = player_node->getInventory()->getItem(index);
                if (!item)
                {
                    tradeWindow->receivedOk(true);
                    return;
                }
                int quantity = msg.readInt16();

                switch (msg.readInt8())
                {
                    case 0:
                        // Successfully added item
                        if (item->isEquipment() && item->isEquipped())
                        {
                            Net::getInventoryHandler()->unequipItem(item);
                        }
                        tradeWindow->addItem(item->getId(), true, quantity,
                                item->isEquipment());
                        item->increaseQuantity(-quantity);
                        break;
                    case 1:
                        // Add item failed - player overweighted
                        localChatTab->chatLog(_("Failed adding item. Trade "
                                "partner is over weighted."), BY_SERVER);
                        break;
                    case 2:
                         // Add item failed - player has no free slot
                         localChatTab->chatLog(_("Failed adding item. Trade "
                                 "partner has no free slot."), BY_SERVER);
                         break;
                    default:
                        localChatTab->chatLog(_("Failed adding item for "
                                "unknown reason."), BY_SERVER);
                        break;
                }
            }
            break;

        case SMSG_TRADE_OK:
            // 0 means ok from myself, 1 means ok from other;
            tradeWindow->receivedOk(msg.readInt8() == 0);
            break;

        case SMSG_TRADE_CANCEL:
            localChatTab->chatLog(_("Trade canceled."), BY_SERVER);
            tradeWindow->setVisible(false);
            tradeWindow->reset();
            player_node->setTrading(false);
            break;

        case SMSG_TRADE_COMPLETE:
            localChatTab->chatLog(_("Trade completed."), BY_SERVER);
            tradeWindow->setVisible(false);
            tradeWindow->reset();
            player_node->setTrading(false);
            break;
    }
}

void TradeHandler::request(Being *being)
{
    MessageOut outMsg(CMSG_TRADE_REQUEST);
    outMsg.writeInt32(being->getId());
}

void TradeHandler::respond(bool accept)
{
    if (!accept)
        player_node->setTrading(false);

    MessageOut outMsg(CMSG_TRADE_RESPONSE);
    outMsg.writeInt8(accept ? 3 : 4);
}

void TradeHandler::addItem(Item *item, int amount)
{
    MessageOut outMsg(CMSG_TRADE_ITEM_ADD_REQUEST);
    outMsg.writeInt16(item->getInvIndex() + INVENTORY_OFFSET);
    outMsg.writeInt32(amount);
}

void TradeHandler::removeItem(int slotNum, int amount)
{
    // TODO
}

void TradeHandler::setMoney(int amount)
{
    MessageOut outMsg(CMSG_TRADE_ITEM_ADD_REQUEST);
    outMsg.writeInt16(0);
    outMsg.writeInt32(amount);
}

void TradeHandler::confirm()
{
    MessageOut outMsg(CMSG_TRADE_ADD_COMPLETE);
}

void TradeHandler::finish()
{
    MessageOut outMsg(CMSG_TRADE_OK);
}

void TradeHandler::cancel()
{
    MessageOut outMsg(CMSG_TRADE_CANCEL_REQUEST);
}

} // namespace EAthena
