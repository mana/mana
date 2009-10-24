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

#include "net/manaserv/gamehandler.h"

#include "net/manaserv/chathandler.h"
#include "net/manaserv/connection.h"
#include "net/manaserv/protocol.h"

#include "net/manaserv/chatserver/chatserver.h"
#include "net/manaserv/gameserver/gameserver.h"

#include "main.h"

Net::GameHandler *gameHandler;

extern ManaServ::ChatHandler *chatHandler;

extern Net::Connection *gameServerConnection;

namespace ManaServ {

extern std::string netToken;
extern ServerInfo gameServer;

GameHandler::GameHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_DISCONNECT_RESPONSE,
        0
    };
    handledMessages = _messages;
    gameHandler = this;
}

void GameHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_DISCONNECT_RESPONSE:
        {
            int errMsg = msg.readInt8();
            // Successful logout
            if (errMsg == ERRMSG_OK)
            {
                // TODO: Handle logout
            }
            // Logout failed
            else
            {
                switch (errMsg) {
                    case ERRMSG_NO_LOGIN:
                        errorMessage = "Gameserver: Not logged in";
                        break;
                    default:
                        errorMessage = "Gameserver: Unknown error";
                        break;
                }
                state = STATE_ERROR;
            }
        }
            break;
    }
}

void GameHandler::connect()
{
    //
}

bool GameHandler::isConnected()
{
    return gameServerConnection->isConnected() &&
            chatHandler->isConnected();
}

void GameHandler::disconnect()
{
    gameServerConnection->disconnect();
    chatHandler->disconnect();
}

void GameHandler::inGame()
{
    Net::GameServer::connect(gameServerConnection, netToken);
    chatHandler->connect();
}

void GameHandler::mapLoaded(const std::string &mapName)
{
    // TODO
}

void GameHandler::who()
{
    // TODO
}

void GameHandler::quit()
{
    // TODO
}

void GameHandler::ping(int tick)
{
    // TODO
}

void GameHandler::clear()
{
    disconnect();
}

} // namespace ManaServ
