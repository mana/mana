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

#include "tradehandler.h"

#include "../messagein.h"
#include "protocol.h"

#include "gameserver/player.h"

#include "../../beingmanager.h"
#include "../../item.h"
#include "../../localplayer.h"

#include "../../gui/chat.h"
#include "../../gui/confirm_dialog.h"
#include "../../gui/trade.h"

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
                Net::GameServer::Player::acceptTrade(false);
        }
    } listener;
}

TradeHandler::TradeHandler():
    mAcceptTradeRequests(true)
{
    static const Uint16 _messages[] = {
        GPMSG_TRADE_REQUEST,
        GPMSG_TRADE_CANCEL,
        GPMSG_TRADE_START,
        GPMSG_TRADE_COMPLETE,
        GPMSG_TRADE_ACCEPT,
        GPMSG_TRADE_ADD_ITEM,
        GPMSG_TRADE_SET_MONEY,
        0
    };
    handledMessages = _messages;
}

void TradeHandler::setAcceptTradeRequests(bool acceptTradeRequests)
{
    mAcceptTradeRequests = acceptTradeRequests;
    if (mAcceptTradeRequests) {
        chatWindow->chatLog("Accepting incoming trade requests", BY_SERVER);
    } else {
        chatWindow->chatLog("Ignoring incoming trade requests", BY_SERVER);
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
            ConfirmDialog *dlg = new ConfirmDialog("Request for trade",
                tradePartnerName + " wants to trade with you, do you accept?");
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
            tradeWindow->setCaption("Trading with " + tradePartnerName);
            tradeWindow->setVisible(true);
            break;

        case GPMSG_TRADE_ACCEPT:
            tradeWindow->receivedOk();
            break;

        case GPMSG_TRADE_CANCEL:
            chatWindow->chatLog("Trade canceled.", BY_SERVER);
            tradeWindow->setVisible(false);
            tradeWindow->reset();
            player_node->setTrading(false);
            break;

        case GPMSG_TRADE_COMPLETE:
            chatWindow->chatLog("Trade completed.", BY_SERVER);
            tradeWindow->setVisible(false);
            tradeWindow->reset();
            player_node->setTrading(false);
            break;
    }
}
