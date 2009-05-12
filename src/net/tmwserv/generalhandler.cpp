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

#include "gui/inventorywindow.h"
#include "gui/partywindow.h"

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

#include "utils/gettext.h"

#include <list>

Net::GeneralHandler *generalHandler;

Net::Connection *gameServerConnection = 0;
Net::Connection *chatServerConnection = 0;
Net::Connection *accountServerConnection = 0;

namespace TmwServ {

GeneralHandler::GeneralHandler():
    mBeingHandler(new BeingHandler),
    mBuySellHandler(new BuySellHandler),
    mCharServerHandler(new CharServerHandler),
    mChatHandler(new ChatHandler),
    mEffectHandler(new EffectHandler),
    mGuildHandler(new GuildHandler),
    mInventoryHandler(new InventoryHandler),
    mItemHandler(new ItemHandler),
    mLoginHandler(new LoginHandler),
    mLogoutHandler(new LogoutHandler),
    mMapHandler(new MapHandler),
    mNpcHandler(new NpcHandler),
    mPartyHandler(new PartyHandler),
    mPlayerHandler(new PlayerHandler),
    mTradeHandler(new TradeHandler)
{
    accountServerConnection = Net::getConnection();
    gameServerConnection = Net::getConnection();
    chatServerConnection = Net::getConnection();

    generalHandler = this;

    std::list<ItemDB::Stat> stats;
    stats.push_back(ItemDB::Stat("str", N_("Strength %+d")));
    stats.push_back(ItemDB::Stat("agi", N_("Agility %+d")));
    stats.push_back(ItemDB::Stat("dex", N_("Dexterity %+d")));
    stats.push_back(ItemDB::Stat("vit", N_("Vitality %+d")));
    stats.push_back(ItemDB::Stat("int", N_("Intelligence %+d")));
    stats.push_back(ItemDB::Stat("will", N_("Willpower %+d")));

    ItemDB::setStatsList(stats);
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
    Net::clearHandlers();

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

void GeneralHandler::tick()
{
    // TODO
}

void GeneralHandler::guiWindowsLoaded()
{
    inventoryWindow->setSplitAllowed(true);
    partyWindow->clearPartyName();
}

void GeneralHandler::guiWindowsUnloaded()
{
    // TODO
}

} // namespace TmwServ
