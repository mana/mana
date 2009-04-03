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

#include "net/ea/maphandler.h"

#include "net/ea/protocol.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "localplayer.h"
#include "log.h"
#include "main.h"

#include "gui/widgets/chattab.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

MapHandler *mapHandler;

MapHandler::MapHandler()
{
    static const Uint16 _messages[] = {
        SMSG_CONNECTION_PROBLEM,
        SMSG_LOGIN_SUCCESS,
        SMSG_SERVER_PING,
        SMSG_WHO_ANSWER,
        0
    };
    handledMessages = _messages;
    mapHandler = this;
}

void MapHandler::handleMessage(MessageIn &msg)
{
    int code;
    unsigned char direction;

    switch (msg.getId())
    {
        case SMSG_CONNECTION_PROBLEM:
            code = msg.readInt8();
            logger->log("Connection problem: %i", code);

            switch (code) {
                case 0:
                    errorMessage = _("Authentication failed");
                    break;
                case 2:
                    errorMessage = _("This account is already logged in");
                    break;
                default:
                    errorMessage = _("Unknown connection error");
                    break;
            }
            state = STATE_ERROR;
            break;

        case SMSG_LOGIN_SUCCESS:
            msg.readInt32();   // server tick
            msg.readCoordinates(player_node->mX, player_node->mY, direction);
            msg.skip(2);      // unknown
            logger->log("Protocol: Player start position: (%d, %d), Direction: %d",
                    player_node->mX, player_node->mY, direction);
            state = STATE_GAME;
            break;

        case SMSG_SERVER_PING:
            // We ignore this for now
            // int tick = msg.readInt32()
            break;

        case SMSG_WHO_ANSWER:
            localChatTab->chatLog("Online users: " + toString(msg.readInt32()),
                    BY_SERVER);
            break;
    }
}

void MapHandler::connect(LoginData *loginData)
{
    // Send login infos
    MessageOut outMsg(CMSG_MAP_SERVER_CONNECT);
    outMsg.writeInt32(loginData->account_ID);
    outMsg.writeInt32(player_node->mCharId);
    outMsg.writeInt32(loginData->session_ID1);
    outMsg.writeInt32(loginData->session_ID2);
    outMsg.writeInt8(loginData->sex);
}

void MapHandler::mapLoaded(const std::string &mapName)
{
    MessageOut outMsg(CMSG_MAP_LOADED);
}

void MapHandler::who()
{
    MessageOut outMsg(0x00c1);
}

void MapHandler::quit()
{
    MessageOut outMsg(CMSG_CLIENT_QUIT);
}

void MapHandler::ping(int tick)
{
    MessageOut msg(CMSG_CLIENT_PING);
    msg.writeInt32(tick);
}
