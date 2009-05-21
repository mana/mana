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

#include "net/tmwserv/tradehandler.h"

#include "net/tmwserv/gameserver/internal.h"
#include "net/tmwserv/gameserver/player.h"

#include "net/tmwserv/connection.h"
#include "net/tmwserv/protocol.h"

#include "net/messagein.h"
#include "net/messageout.h"
#include "net/net.h"

#include "beingmanager.h"
#include "item.h"
#include "localplayer.h"

#include "gui/confirmdialog.h"
#include "gui/trade.h"

#include "gui/widgets/chattab.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

std::string tradePartnerName;
int tradePartnerID;

/**
 * Listener for request trade dialogs
 */
namespace {
    struct RequestTradeListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            if (event.getId() == "yes")
                Net::GameServer::Player::requestTrade(tradePartnerID);
            else
                Net::getTradeHandler()->cancel();
        }
    } listener;
}

Net::TradeHandler *tradeHandler;

namespace TmwServ {

TradeHandler::TradeHandler():
    mAcceptTradeRequests(true)
{
    static const Uint16 _messages[] = {
        GPMSG_TRADE_REQUEST,
        GPMSG_TRADE_CANCEL,
        GPMSG_TRADE_START,
        GPMSG_TRADE_COMPLETE,
        GPMSG_TRADE_AGREED,
        GPMSG_TRADE_BOTH_CONFIRM,
        GPMSG_TRADE_CONFIRM,
        GPMSG_TRADE_ADD_ITEM,
        GPMSG_TRADE_SET_MONEY,
        0
    };
    handledMessages = _messages;
    tradeHandler = this;
    
}

void TradeHandler::setAcceptTradeRequests(bool acceptTradeRequests)
{
    mAcceptTradeRequests = acceptTradeRequests;
    if (mAcceptTradeRequests) {
        localChatTab->chatLog(_("Accepting incoming trade requests."), BY_SERVER);
    } else {
        localChatTab->chatLog(_("Ignoring incoming trade requests."), BY_SERVER);
    }
}

void TradeHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_TRADE_REQUEST:
        {
            Being *being = beingManager->findBeing(msg.readInt16());
            if (!being || !mAcceptTradeRequests)
            {
                Net::GameServer::Player::acceptTrade(false);
                break;
            }
            player_node->setTrading(true);
            tradePartnerName = being->getName();
            tradePartnerID = being->getId();
            ConfirmDialog *dlg = new ConfirmDialog(_("Request for Trade"),
                    strprintf(_("%s wants to trade with you, do you accept?"),
                            tradePartnerName.c_str()));
            dlg->addActionListener(&listener);
        }   break;

        case GPMSG_TRADE_ADD_ITEM:
        {
            int type = msg.readInt16();
            int amount = msg.readInt8();
            tradeWindow->addItem(type, false, amount);
        }   break;

        case GPMSG_TRADE_SET_MONEY:
            tradeWindow->setMoney(msg.readInt32());
            break;

        case GPMSG_TRADE_START:
            tradeWindow->reset();
            tradeWindow->setCaption(strprintf(_("Trading with %s"),
                                              tradePartnerName.c_str()));
            tradeWindow->setVisible(true);
            break;

        case GPMSG_TRADE_BOTH_CONFIRM:
            tradeWindow->receivedOk(false);
            break;

        case GPMSG_TRADE_AGREED:
            tradeWindow->receivedOk(false);
            break;
            
        case GPMSG_TRADE_CANCEL:
            localChatTab->chatLog(_("Trade canceled."), BY_SERVER);
            tradeWindow->setVisible(false);
            tradeWindow->reset();
            player_node->setTrading(false);
            break;

        case GPMSG_TRADE_COMPLETE:
            localChatTab->chatLog(_("Trade completed."), BY_SERVER);
            tradeWindow->setVisible(false);
            tradeWindow->reset();
            player_node->setTrading(false);
            break;
    }
}

void TradeHandler::request(Being *being)
{
    tradePartnerName = being->getName();
    tradePartnerID = being->getId();

    MessageOut msg(PGMSG_TRADE_REQUEST);
    msg.writeInt16(tradePartnerID);
    Net::GameServer::connection->send(msg);
}

void TradeHandler::respond(bool accept)
{
    // TODO
    if (!accept)
        player_node->setTrading(false);
}

void TradeHandler::addItem(Item *item, int amount)
{
    MessageOut msg(PGMSG_TRADE_ADD_ITEM);
    msg.writeInt8(item->getInvIndex());
    msg.writeInt8(amount);
    Net::GameServer::connection->send(msg);

    tradeWindow->addItem(item->getId(), true, amount);
    item->increaseQuantity(-amount);
}

void TradeHandler::removeItem(int slotNum, int amount)
{
    // TODO
}

void TradeHandler::setMoney(int amount)
{
    MessageOut msg(PGMSG_TRADE_SET_MONEY);
    msg.writeInt32(amount);
    Net::GameServer::connection->send(msg);
}

void TradeHandler::confirm()
{
    MessageOut msg(PGMSG_TRADE_CONFIRM);
    Net::GameServer::connection->send(msg);
}

void TradeHandler::finish()
{
    MessageOut msg(PGMSG_TRADE_AGREED);
    Net::GameServer::connection->send(msg);
}

void TradeHandler::cancel()
{
    MessageOut msg(PGMSG_TRADE_CANCEL);
    Net::GameServer::connection->send(msg);
}

} // namespace TmwServ
