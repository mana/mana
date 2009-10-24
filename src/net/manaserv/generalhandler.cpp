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

#include "net/manaserv/generalhandler.h"

#include "gui/changeemaildialog.h"
#include "gui/charselectdialog.h"
#include "gui/inventorywindow.h"
#include "gui/partywindow.h"
#include "gui/register.h"
#include "gui/skilldialog.h"
#include "gui/specialswindow.h"
#include "gui/statuswindow.h"

#include "net/manaserv/network.h"
#include "net/manaserv/connection.h"

#include "net/manaserv/beinghandler.h"
#include "net/manaserv/buysellhandler.h"
#include "net/manaserv/charhandler.h"
#include "net/manaserv/chathandler.h"
#include "net/manaserv/effecthandler.h"
#include "net/manaserv/gamehandler.h"
#include "net/manaserv/guildhandler.h"
#include "net/manaserv/inventoryhandler.h"
#include "net/manaserv/itemhandler.h"
#include "net/manaserv/loginhandler.h"
#include "net/manaserv/npchandler.h"
#include "net/manaserv/partyhandler.h"
#include "net/manaserv/playerhandler.h"
#include "net/manaserv/specialhandler.h"
#include "net/manaserv/tradehandler.h"

#include "utils/gettext.h"

#include "main.h"

#include <list>

Net::GeneralHandler *generalHandler  = NULL;

Net::Connection *gameServerConnection = 0;
Net::Connection *chatServerConnection = 0;
Net::Connection *accountServerConnection = 0;

namespace ManaServ {

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
    Net::initialize();

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
    Net::registerHandler(mCharHandler.get());
    Net::registerHandler(mChatHandler.get());
    Net::registerHandler(mEffectHandler.get());
    Net::registerHandler(mGameHandler.get());
    Net::registerHandler(mGuildHandler.get());
    Net::registerHandler(mInventoryHandler.get());
    Net::registerHandler(mItemHandler.get());
    Net::registerHandler(mLoginHandler.get());
    Net::registerHandler(mNpcHandler.get());
    Net::registerHandler(mPartyHandler.get());
    Net::registerHandler(mPlayerHandler.get());
    Net::registerHandler(mTradeHandler.get());
}

void GeneralHandler::reload()
{
    // Nothing needed yet
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
    skillDialog->loadSkills("skills.xml");
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
    Net::clearHandlers();
}

} // namespace ManaServ
