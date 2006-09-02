/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include "charserverhandler.h"

#include "messagein.h"
#include "network.h"
#include "protocol.h"

#include "../game.h"
#include "../localplayer.h"
#include "../log.h"
#include "../logindata.h"
#include "../main.h"

#include "../gui/ok_dialog.h"

CharServerHandler::CharServerHandler()
{
    static const Uint16 _messages[] = {
        APMSG_CHAR_CREATE_RESPONSE,
        APMSG_CHAR_DELETE_RESPONSE,
        APMSG_CHAR_INFO,
        APMSG_CHAR_SELECT_RESPONSE,
        0
    };
    handledMessages = _messages;
}

void
CharServerHandler::handleMessage(MessageIn &msg)
{
    int slot;
    LocalPlayer *tempPlayer;

    switch (msg.getId())
    {
        case APMSG_CHAR_CREATE_RESPONSE:
            handleCharCreateResponse(msg);
            break;

        case APMSG_CHAR_DELETE_RESPONSE:
        {
            int errMsg = msg.readByte();
            // Character deletion successful
            if (errMsg == ERRMSG_OK)
            {
                delete mCharInfo->getEntry();
                mCharInfo->setEntry(0);
                mCharInfo->unlock();
                n_character--;
                new OkDialog("Info", "Player deleted");
            }
            // Character deletion failed
            else
            {
                std::string message = "";
                switch (errMsg)
                {
                    case ERRMSG_NO_LOGIN:
                        message = "Not logged in";
                        break;
                    case ERRMSG_INVALID_ARGUMENT:
                        message = "Selection out of range";
                        break;
                    default:
                        message = "Unknown error";
                }
                mCharInfo->unlock();
                new OkDialog("Error", message);
            }
        }
            break;

        case APMSG_CHAR_INFO:
            tempPlayer = readPlayerData(msg, slot);
            mCharInfo->unlock();
            mCharInfo->select(slot);
            mCharInfo->setEntry(tempPlayer);
            n_character++;
            break;

        case APMSG_CHAR_SELECT_RESPONSE:
            handleCharSelectResponse(msg);
            break;
    }
}

void
CharServerHandler::handleCharCreateResponse(MessageIn &msg)
{
    int errMsg = msg.readByte();

    // Character creation failed
    if (errMsg != ERRMSG_OK)
    {
        std::string message = "";
        switch (errMsg)
        {
            case ERRMSG_NO_LOGIN:
                message = "Not logged in";
                break;
            case CREATE_TOO_MUCH_CHARACTERS:
                message = "No empty slot";
                break;
            case ERRMSG_INVALID_ARGUMENT:
                message = "Invalid name";
                break;
            case CREATE_EXISTS_NAME:
                message = "Character's name already exists";
                break;
            case CREATE_INVALID_HAIRSTYLE:
                message = "Invalid hairstyle";
                break;
            case CREATE_INVALID_HAIRCOLOR:
                message = "Invalid hair color";
                break;
            case CREATE_INVALID_GENDER:
                message = "Invalid gender";
                break;
            case CREATE_RAW_STATS_TOO_HIGH:
                message = "Character's stats are too high";
                break;
            case CREATE_RAW_STATS_TOO_LOW:
                message = "Character's stats are too low";
                break;
            case CREATE_RAW_STATS_INVALID_DIFF:
                message = "Character's stats difference is too high";
                break;
            case CREATE_RAW_STATS_EQUAL_TO_ZERO:
                message = "One stat is zero";
                break;
            default:
                message = "Unknown error";
                break;
        }
        new OkDialog("Error", message);
    }
}

void
CharServerHandler::handleCharSelectResponse(MessageIn &msg)
{
    int errMsg = msg.readByte();

    if (errMsg == ERRMSG_OK)
    {
        token = msg.readString(32);
        std::string gameServer = msg.readString();
        unsigned short gameServerPort = msg.readShort();
        std::string chatServer = msg.readString();
        unsigned short chatServerPort = msg.readShort();

        logger->log("Game server: %s:%d", gameServer.c_str(), gameServerPort);
        logger->log("Chat server: %s:%d", chatServer.c_str(), chatServerPort);

        Network::connect(Network::GAME, gameServer, gameServerPort);
        Network::connect(Network::CHAT, chatServer, chatServerPort);

        // Keep the selected character and delete the others
        player_node = mCharInfo->getEntry();
        mCharInfo->unlock();
        mCharInfo->select(0);
        do {
            LocalPlayer *tmp = mCharInfo->getEntry();
            if (tmp != player_node)
                delete tmp;
            mCharInfo->next();
        } while (mCharInfo->getPos());

        state = STATE_CONNECT_GAME;
    }
}

LocalPlayer*
CharServerHandler::readPlayerData(MessageIn &msg, int &slot)
{
    LocalPlayer *tempPlayer = new LocalPlayer;
    slot = msg.readByte(); // character slot
    tempPlayer->mName = msg.readString();
    tempPlayer->setSex(msg.readByte());
    tempPlayer->setHairStyle(msg.readByte());
    tempPlayer->setHairColor(msg.readByte());
    tempPlayer->mLevel = msg.readByte();
    tempPlayer->mMoney = msg.readShort();
    for (int i = 0; i < 6; i++) {
        tempPlayer->mAttr[i] = msg.readByte();
    }
    return tempPlayer;
}
