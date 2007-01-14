/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include "tradehandler.h"

#include "messagein.h"
#include "protocol.h"

#include "../item.h"
#include "../localplayer.h"

#include "../gui/chat.h"
#include "../gui/confirm_dialog.h"
#include "../gui/trade.h"

std::string tradePartnerName;

/**
 * Listener for request trade dialogs
 */
namespace {
    struct RequestTradeListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            player_node->tradeReply(event.getId() == "yes");
        };
    } listener;
}

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
                if (!player_node->tradeRequestOk())
                {
                    player_node->tradeReply(false);
                    break;
                }

                player_node->setTrading(true);
                tradePartnerName = msg.readString(24);
                ConfirmDialog *dlg;
                dlg = new ConfirmDialog("Request for trade",
                        tradePartnerName +
                        " wants to trade with you, do you accept?");
                dlg->addActionListener(&listener);
            break;

        case SMSG_TRADE_RESPONSE:
            switch (msg.readByte())
            {
                case 0: // Too far away
                    chatWindow->chatLog("Trading isn't possible. "
                            "Trade partner is too far away.",
                            BY_SERVER);
                    break;
                case 1: // Character doesn't exist
                    chatWindow->chatLog("Trading isn't possible. "
                            "Character doesn't exist.",
                            BY_SERVER);
                    break;
                case 2: // Invite request check failed...
                    chatWindow->chatLog("Trade cancelled due to an "
                            "unknown reason.", BY_SERVER);
                    break;
                case 3: // Trade accepted
                    tradeWindow->reset();
                    tradeWindow->setCaption(
                            "Trade: You and " + tradePartnerName);
                    tradeWindow->setVisible(true);
                    break;
                case 4: // Trade cancelled
                    chatWindow->chatLog("Trade cancelled.", BY_SERVER);
                    tradeWindow->setVisible(false);
                    player_node->setTrading(false);
                    break;
                default: // Shouldn't happen as well, but to be sure
                    chatWindow->chatLog("Unhandled trade cancel packet",
                            BY_SERVER);
                    break;
            }
            break;

        case SMSG_TRADE_ITEM_ADD:
            {
                Sint32 amount = msg.readLong();
                Sint16 type = msg.readShort();
                msg.readByte();  // identified flag
                msg.readByte();  // attribute
                msg.readByte();  // refine

                // TODO: handle also identified, etc
                if (type == 0) {
                    tradeWindow->addMoney(amount);
                } else {
                    tradeWindow->addItem(type, false, amount, false);
                }
            }
            break;

        case SMSG_TRADE_ITEM_ADD_RESPONSE:
            // Trade: New Item add response (was 0x00ea, now 01b1)
            {
                Item *item = player_node->getInvItem(msg.readShort());
                Sint16 quantity = msg.readShort();

                switch (msg.readByte())
                {
                    case 0:
                        // Successfully added item
                        if (item->isEquipment() && item->isEquipped())
                        {
                            player_node->unequipItem(item);
                        }
                        tradeWindow->addItem(item->getId(), true, quantity,
                                item->isEquipment());
                        item->increaseQuantity(-quantity);
                        break;
                    case 1:
                        // Add item failed - player overweighted
                        chatWindow->chatLog("Failed adding item. Trade "
                                "partner is over weighted.",
                                BY_SERVER);
                        break;
                    default:
                        chatWindow->chatLog("Failed adding item for "
                                "unknown reason.", BY_SERVER);
                        break;
                }
            }
            break;

        case SMSG_TRADE_OK:
            // 0 means ok from myself, 1 means ok from other;
            tradeWindow->receivedOk(msg.readByte() == 0);
            break;

        case SMSG_TRADE_CANCEL:
            chatWindow->chatLog("Trade canceled.", BY_SERVER);
            tradeWindow->setVisible(false);
            tradeWindow->reset();
            player_node->setTrading(false);
            break;

        case SMSG_TRADE_COMPLETE:
            chatWindow->chatLog("Trade completed.", BY_SERVER);
            tradeWindow->setVisible(false);
            tradeWindow->reset();
            player_node->setTrading(false);
            break;
    }
}
