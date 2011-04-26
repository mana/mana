/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "net/manaserv/gamehandler.h"

#include "client.h"
#include "localplayer.h"

#include "net/manaserv/chathandler.h"
#include "net/manaserv/connection.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/manaserv_protocol.h"

extern Net::GameHandler *gameHandler;

extern ManaServ::ChatHandler *chatHandler;

namespace ManaServ {

extern Connection *chatServerConnection;
extern Connection *gameServerConnection;
extern std::string netToken;
extern ServerInfo gameServer;
extern ServerInfo chatServer;

GameHandler::GameHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_DISCONNECT_RESPONSE,
        0
    };
    handledMessages = _messages;
    gameHandler = this;
}

void GameHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_DISCONNECT_RESPONSE:
        {
            int errMsg = msg.readInt8();
            // Successful logout
            if (errMsg == ERRMSG_OK)
            {
                netToken = msg.readString(32);

                if (!netToken.empty())
                {
                    Client::setState(STATE_SWITCH_CHARACTER);
                }
                else
                {
                    // TODO: Handle logout
                }
            }
            // Logout failed
            else
            {
                switch (errMsg)
                {
                    case ERRMSG_NO_LOGIN:
                        errorMessage = "Gameserver: Not logged in";
                        break;
                    default:
                        errorMessage = "Gameserver: Unknown error";
                        break;
                }
                Client::setState(STATE_ERROR);
            }
        }
            break;
    }
}

void GameHandler::connect()
{
    gameServerConnection->connect(gameServer.hostname, gameServer.port);

    // Will already be connected if we just changed gameservers
    if (!chatServerConnection->isConnected())
        chatServerConnection->connect(chatServer.hostname, chatServer.port);
}

bool GameHandler::isConnected()
{
    return gameServerConnection->isConnected() &&
            chatHandler->isConnected();
}

void GameHandler::disconnect()
{
    gameServerConnection->disconnect();

    // No need if we're just changing gameservers
    if (Client::getState() != STATE_CHANGE_MAP)
        chatHandler->disconnect();
}

void GameHandler::who()
{
    // TODO
}

void GameHandler::quit(bool reconnectAccount)
{
    MessageOut msg(PGMSG_DISCONNECT);
    msg.writeInt8((unsigned char) reconnectAccount);
    gameServerConnection->send(msg);
}

void GameHandler::ping(int tick)
{
    // TODO
}

void GameHandler::gameLoading()
{
    MessageOut msg(PGMSG_CONNECT);
    msg.writeString(netToken, 32);
    gameServerConnection->send(msg);

    chatHandler->connect();

    // Attack range from item DB
    // TODO: Deharcode it through equipment handling
    player_node->setAttackRange(48);
}

} // namespace ManaServ
