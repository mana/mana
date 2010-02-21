/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "net/manaserv/generalhandler.h"

#include "main.h"

#include "gui/changeemaildialog.h"
#include "gui/charselectdialog.h"
#include "gui/inventorywindow.h"
#include "gui/register.h"
#include "gui/skilldialog.h"
#include "gui/specialswindow.h"
#include "gui/statuswindow.h"

#include "net/manaserv/beinghandler.h"
#include "net/manaserv/buysellhandler.h"
#include "net/manaserv/charhandler.h"
#include "net/manaserv/chathandler.h"
#include "net/manaserv/connection.h"
#include "net/manaserv/effecthandler.h"
#include "net/manaserv/gamehandler.h"
#include "net/manaserv/guildhandler.h"
#include "net/manaserv/inventoryhandler.h"
#include "net/manaserv/itemhandler.h"
#include "net/manaserv/loginhandler.h"
#include "net/manaserv/network.h"
#include "net/manaserv/npchandler.h"
#include "net/manaserv/partyhandler.h"
#include "net/manaserv/playerhandler.h"
#include "net/manaserv/specialhandler.h"
#include "net/manaserv/tradehandler.h"

#include "utils/gettext.h"

#include <list>

extern Net::GeneralHandler *generalHandler;

extern ManaServ::LoginHandler *loginHandler;

namespace ManaServ {

Connection *accountServerConnection = 0;
Connection *chatServerConnection = 0;
Connection *gameServerConnection = 0;
std::string netToken = "";
ServerInfo gameServer;
ServerInfo chatServer;

GeneralHandler::GeneralHandler():
        mBeingHandler(new BeingHandler),
        mBuySellHandler(new BuySellHandler),
        mCharHandler(new CharHandler),
        mChatHandler(new ChatHandler),
        mEffectHandler(new EffectHandler),
        mGameHandler(new GameHandler),
        mGuildHandler(new GuildHandler),
        mInventoryHandler(new InventoryHandler),
        mItemHandler(new ItemHandler),
        mLoginHandler(new LoginHandler),
        mNpcHandler(new NpcHandler),
        mPartyHandler(new PartyHandler),
        mPlayerHandler(new PlayerHandler),
        mTradeHandler(new TradeHandler),
        mSpecialHandler(new SpecialHandler)
{
    initialize();

    accountServerConnection = getConnection();
    gameServerConnection = getConnection();
    chatServerConnection = getConnection();

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
    registerHandler(mBeingHandler.get());
    registerHandler(mBuySellHandler.get());
    registerHandler(mCharHandler.get());
    registerHandler(mChatHandler.get());
    registerHandler(mEffectHandler.get());
    registerHandler(mGameHandler.get());
    registerHandler(mGuildHandler.get());
    registerHandler(mInventoryHandler.get());
    registerHandler(mItemHandler.get());
    registerHandler(mLoginHandler.get());
    registerHandler(mNpcHandler.get());
    registerHandler(mPartyHandler.get());
    registerHandler(mPlayerHandler.get());
    registerHandler(mTradeHandler.get());
}

void GeneralHandler::reload()
{
    // Nothing needed yet
}

void GeneralHandler::unload()
{
    clearHandlers();

    if (accountServerConnection)
        accountServerConnection->disconnect();
    if (gameServerConnection)
        gameServerConnection->disconnect();
    if (chatServerConnection)
        chatServerConnection->disconnect();

    delete accountServerConnection;
    delete gameServerConnection;
    delete chatServerConnection;

    finalize();
}

void GeneralHandler::flushNetwork()
{
    flush();

    if (state == STATE_SWITCH_CHARACTER &&
        Net::getLoginHandler()->isConnected())
    {
        loginHandler->reconnect();
        state = STATE_GET_CHARACTERS;
    }
}

void GeneralHandler::guiWindowsLoaded()
{
    inventoryWindow->setSplitAllowed(true);
    skillDialog->loadSkills("tmw-skills.xml");
    specialsWindow->loadSpecials("specials.xml");

    player_node->setExpNeeded(100);

    statusWindow->addAttribute(16, _("Strength"), true);
    statusWindow->addAttribute(17, _("Agility"), true);
    statusWindow->addAttribute(18, _("Dexterity"), true);
    statusWindow->addAttribute(19, _("Vitality"), true);
    statusWindow->addAttribute(20, _("Intelligence"), true);
    statusWindow->addAttribute(21, _("Willpower"), true);
}

void GeneralHandler::guiWindowsUnloaded()
{
    // TODO
}

void GeneralHandler::clearHandlers()
{
    clearNetworkHandlers();
}

} // namespace ManaServ
