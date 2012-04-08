/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "net/manaserv/tradehandler.h"

#include "actorspritemanager.h"
#include "event.h"
#include "item.h"
#include "localplayer.h"
#include "playerinfo.h"

#include "gui/confirmdialog.h"
#include "gui/tradewindow.h"

#include "net/net.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/manaserv_protocol.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

extern std::string tradePartnerName;
int tradePartnerID;

extern Net::TradeHandler *tradeHandler;

namespace ManaServ {

extern Connection *gameServerConnection;

/**
 * Listener for request trade dialogs
 */
namespace {
    struct RequestTradeListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            if (event.getId() == "yes")
            {
                ManaServ::MessageOut msg(PGMSG_TRADE_REQUEST);
                msg.writeInt16(tradePartnerID);
                gameServerConnection->send(msg);
            }
            else
                Net::getTradeHandler()->cancel();
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
    if (mAcceptTradeRequests)
        SERVER_NOTICE(_("Accepting incoming trade requests."))
    else
        SERVER_NOTICE(_("Ignoring incoming trade requests."))
}

void TradeHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_TRADE_REQUEST:
        {
            Being *being = actorSpriteManager->findBeing(msg.readInt16());
            if (!being || !mAcceptTradeRequests)
            {
                respond(false);
                break;
            }
            mTrading = true;
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
            SERVER_NOTICE(_("Trade canceled."))
            tradeWindow->setVisible(false);
            tradeWindow->reset();
            mTrading = false;
            break;

        case GPMSG_TRADE_COMPLETE:
            SERVER_NOTICE(_("Trade completed."))
            tradeWindow->setVisible(false);
            tradeWindow->reset();
            mTrading = false;
            break;
    }
}

void TradeHandler::request(Being *being)
{
    tradePartnerName = being->getName();
    tradePartnerID = being->getId();

    MessageOut msg(PGMSG_TRADE_REQUEST);
    msg.writeInt16(tradePartnerID);
    gameServerConnection->send(msg);
}

void TradeHandler::respond(bool accept)
{
    MessageOut msg(accept ? PGMSG_TRADE_REQUEST : PGMSG_TRADE_CANCEL);
    gameServerConnection->send(msg);

    if (!accept)
        mTrading = false;
}

void TradeHandler::addItem(Item *item, int amount)
{
    MessageOut msg(PGMSG_TRADE_ADD_ITEM);
    msg.writeInt8(item->getInvIndex());
    msg.writeInt8(amount);
    gameServerConnection->send(msg);

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
    gameServerConnection->send(msg);
}

void TradeHandler::confirm()
{
    MessageOut msg(PGMSG_TRADE_CONFIRM);
    gameServerConnection->send(msg);
}

void TradeHandler::finish()
{
    MessageOut msg(PGMSG_TRADE_AGREED);
    gameServerConnection->send(msg);
}

void TradeHandler::cancel()
{
    MessageOut msg(PGMSG_TRADE_CANCEL);
    gameServerConnection->send(msg);
}

} // namespace ManaServ
