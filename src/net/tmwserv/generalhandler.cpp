/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "net/tmwserv/generalhandler.h"

#include "net/tmwserv/network.h"
#include "net/tmwserv/connection.h"

#include "net/tmwserv/beinghandler.h"
#include "net/tmwserv/buysellhandler.h"
#include "net/tmwserv/charserverhandler.h"
#include "net/tmwserv/chathandler.h"
#include "net/tmwserv/effecthandler.h"
#include "net/tmwserv/guildhandler.h"
#include "net/tmwserv/inventoryhandler.h"
#include "net/tmwserv/itemhandler.h"
#include "net/tmwserv/loginhandler.h"
#include "net/tmwserv/logouthandler.h"
#include "net/tmwserv/maphandler.h"
#include "net/tmwserv/npchandler.h"
#include "net/tmwserv/partyhandler.h"
#include "net/tmwserv/playerhandler.h"
#include "net/tmwserv/tradehandler.h"

Net::GeneralHandler *generalHandler;

Net::Connection *gameServerConnection = 0;
Net::Connection *chatServerConnection = 0;
Net::Connection *accountServerConnection = 0;

namespace TmwServ {

GeneralHandler::GeneralHandler():
    mBeingHandler(new BeingHandler),
    mBuySellHandler(new BuySellHandler),
    mCharServerHandler(new TmwServ::CharServerHandler),
    mChatHandler(new TmwServ::ChatHandler),
    mEffectHandler(new EffectHandler),
    mGuildHandler(new GuildHandler),
    mInventoryHandler(new TmwServ::InventoryHandler),
    mItemHandler(new ItemHandler),
    mLoginHandler(new TmwServ::LoginHandler),
    mLogoutHandler(new LogoutHandler),
    mMapHandler(new TmwServ::MapHandler),
    mNpcHandler(new TmwServ::NpcHandler),
    mPartyHandler(new PartyHandler),
    mPlayerHandler(new TmwServ::PlayerHandler),
    mTradeHandler(new TmwServ::TradeHandler)
{
    accountServerConnection = Net::getConnection();
    gameServerConnection = Net::getConnection();
    chatServerConnection = Net::getConnection();

    generalHandler = this;
}

void GeneralHandler::load()
{
    Net::registerHandler(mBeingHandler.get());
    Net::registerHandler(mBuySellHandler.get());
    Net::registerHandler(mCharServerHandler.get());
    Net::registerHandler(mChatHandler.get());
    Net::registerHandler(mEffectHandler.get());
    Net::registerHandler(mGuildHandler.get());
    Net::registerHandler(mInventoryHandler.get());
    Net::registerHandler(mItemHandler.get());
    Net::registerHandler(mLoginHandler.get());
    Net::registerHandler(mLogoutHandler.get());
    Net::registerHandler(mMapHandler.get());
    Net::registerHandler(mNpcHandler.get());
    Net::registerHandler(mPartyHandler.get());
    Net::registerHandler(mPlayerHandler.get());
    Net::registerHandler(mTradeHandler.get());
}

void GeneralHandler::unload()
{
    if (accountServerConnection)
        accountServerConnection->disconnect();
    if (gameServerConnection)
        gameServerConnection->disconnect();
    if (chatServerConnection)
        chatServerConnection->disconnect();

    delete accountServerConnection;
    delete gameServerConnection;
    delete chatServerConnection;

    Net::finalize();
}

void GeneralHandler::flushNetwork()
{
    Net::flush();
}

bool GeneralHandler::isNetworkConnected()
{
    // TODO
    return true;
}

void GeneralHandler::guiWindowsLoaded()
{
    // TODO
}

} // namespace TmwServ
