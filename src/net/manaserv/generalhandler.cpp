/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2013  The Mana Developers
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

#include "client.h"

#include "gui/skilldialog.h"

#include "net/manaserv/adminhandler.h"
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
#include "net/manaserv/abilityhandler.h"
#include "net/manaserv/tradehandler.h"

#include "resources/attributes.h"

extern ManaServ::LoginHandler *loginHandler;

namespace ManaServ {

Connection *accountServerConnection = nullptr;
Connection *chatServerConnection = nullptr;
Connection *gameServerConnection = nullptr;
std::string netToken;
ServerInfo gameServer;
ServerInfo chatServer;

GeneralHandler::GeneralHandler():
        mAdminHandler(new AdminHandler),
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
        mAbilityHandler(new AbilityHandler)
{
    initialize();

    accountServerConnection = getConnection();
    gameServerConnection = getConnection();
    chatServerConnection = getConnection();

    listen(Event::ClientChannel);
    listen(Event::GameChannel);
}

void GeneralHandler::load()
{
    registerHandler(mAdminHandler.get());
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
    static_cast<CharHandler*>(Net::getCharHandler())->clear();

    if (accountServerConnection)
        accountServerConnection->disconnect();

    if (gameServerConnection)
        gameServerConnection->disconnect();

    if (chatServerConnection)
        chatServerConnection->disconnect();

    netToken.clear();
    gameServer.clear();
    chatServer.clear();
}

void GeneralHandler::unload()
{
    clearNetworkHandlers();

    if (accountServerConnection)
    {
        accountServerConnection->disconnect();
        delete accountServerConnection;
        accountServerConnection = nullptr;
    }

    if (gameServerConnection)
    {
        gameServerConnection->disconnect();
        delete gameServerConnection;
        gameServerConnection = nullptr;
    }

    if (chatServerConnection)
    {
        chatServerConnection->disconnect();
        delete chatServerConnection;
        chatServerConnection = nullptr;
    }

    finalize();
}

void GeneralHandler::flushNetwork()
{
    flush();

    if (Client::getState() == STATE_SWITCH_CHARACTER &&
        Net::getLoginHandler()->isConnected())
    {
        loginHandler->reconnect();
        Client::setState(STATE_GET_CHARACTERS);
    }
}

void GeneralHandler::event(Event::Channel channel,
                           const Event &event)
{
    if (channel == Event::ClientChannel)
    {
        if (event.getType() == Event::StateChange)
        {
            int newState = event.getInt("newState");

            if (newState == STATE_GAME)
            {
                auto *game = static_cast<GameHandler*>(Net::getGameHandler());
                game->gameLoading();
            }
        }
    }
    else if (channel == Event::GameChannel)
    {
        if (event.getType() == Event::GuiWindowsLoaded)
        {
            skillDialog->loadSkills();

            PlayerInfo::setAttribute(EXP_NEEDED, 100);

            Attributes::informStatusWindow();
        }
    }
}

} // namespace ManaServ
