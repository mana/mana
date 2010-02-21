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

#include "net/ea/gamehandler.h"

#include "game.h"
#include "localplayer.h"
#include "log.h"
#include "main.h"

#include "gui/widgets/chattab.h"

#include "gui/okdialog.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "net/ea/loginhandler.h"
#include "net/ea/network.h"
#include "net/ea/protocol.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

extern Net::GameHandler *gameHandler;

namespace EAthena {

extern ServerInfo mapServer;

GameHandler::GameHandler()
{
    static const Uint16 _messages[] = {
        SMSG_MAP_LOGIN_SUCCESS,
        SMSG_SERVER_PING,
        SMSG_WHO_ANSWER,
        SMSG_CHAR_SWITCH_RESPONSE,
        SMSG_MAP_QUIT_RESPONSE,
        0
    };
    handledMessages = _messages;
    gameHandler = this;
}

void GameHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_MAP_LOGIN_SUCCESS:
        {
            unsigned char direction;
            Uint16 x, y;
            msg.readInt32();   // server tick
            msg.readCoordinates(x, y, direction);
            msg.skip(2);      // unknown
            logger->log("Protocol: Player start position: (%d, %d), Direction: %d",
                    x, y, direction);
            // Switch now or we'll have problems
            state = STATE_GAME;
            player_node->setTileCoords(x, y);
         }  break;

        case SMSG_SERVER_PING:
            // We ignore this for now
            // int tick = msg.readInt32()
            break;

        case SMSG_WHO_ANSWER:
            localChatTab->chatLog(strprintf(_("Online users: %d"),
                                            msg.readInt32()), BY_SERVER);
            break;

        case SMSG_CHAR_SWITCH_RESPONSE:
            if (msg.readInt8())
            {
                state = STATE_SWITCH_CHARACTER;
            }
            break;

        case SMSG_MAP_QUIT_RESPONSE:
            if (msg.readInt8())
            {
                new OkDialog(_("Game"), _("Request to quit denied!"), NULL);
            }
            break;
    }
}

void GameHandler::connect()
{
    mNetwork->connect(mapServer);

    const Token &token =
            static_cast<LoginHandler*>(Net::getLoginHandler())->getToken();

    // Send login infos
    MessageOut outMsg(CMSG_MAP_SERVER_CONNECT);
    outMsg.writeInt32(token.account_ID);
    outMsg.writeInt32(player_node->getId());
    outMsg.writeInt32(token.session_ID1);
    outMsg.writeInt32(token.session_ID2);
    outMsg.writeInt8((token.sex == GENDER_MALE) ? 1 : 0);

    // Change the player's ID to the account ID to match what eAthena uses
    player_node->setId(token.account_ID);

    // We get 4 useless bytes before the real answer comes in (what are these?)
    mNetwork->skip(4);
}

bool GameHandler::isConnected()
{
    return mNetwork->isConnected();
}

void GameHandler::disconnect()
{
    mNetwork->disconnect();
}

void GameHandler::inGame()
{
    // TODO
}

void GameHandler::mapLoaded(const std::string &mapName)
{
    MessageOut outMsg(CMSG_MAP_LOADED);
}

void GameHandler::who()
{
}

void GameHandler::quit()
{
    MessageOut outMsg(CMSG_CLIENT_QUIT);
}

void GameHandler::ping(int tick)
{
    MessageOut msg(CMSG_CLIENT_PING);
    msg.writeInt32(tick);
}

} // namespace EAthena
