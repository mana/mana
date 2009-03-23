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
 */

#include "charserverhandler.h"

#include "connection.h"
#include "../messagein.h"
#include "protocol.h"

#include "../../game.h"
#include "../../localplayer.h"
#include "../../log.h"
#include "../../logindata.h"
#include "../../main.h"

#include "../../gui/ok_dialog.h"
#include "../../gui/char_select.h"

extern Net::Connection *gameServerConnection;
extern Net::Connection *chatServerConnection;

CharServerHandler::CharServerHandler():
    mCharCreateDialog(0)
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

void CharServerHandler::handleMessage(MessageIn &msg)
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
            int errMsg = msg.readInt8();
            // Character deletion successful
            if (errMsg == ERRMSG_OK)
            {
                delete mCharInfo->getEntry();
                mCharInfo->setEntry(0);
                mCharInfo->unlock();
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

            // Close the character create dialog
            if (mCharCreateDialog)
            {
                mCharCreateDialog->scheduleDelete();
                mCharCreateDialog = 0;
            }
            break;

        case APMSG_CHAR_SELECT_RESPONSE:
            handleCharSelectResponse(msg);
            break;
    }
}

void CharServerHandler::handleCharCreateResponse(MessageIn &msg)
{
    int errMsg = msg.readInt8();

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
            case CREATE_RAW_STATS_EQUAL_TO_ZERO:
                message = "One stat is zero";
                break;
            default:
                message = "Unknown error";
                break;
        }
        new OkDialog("Error", message);
    }

    if (mCharCreateDialog)
        mCharCreateDialog->unlock();
}

void CharServerHandler::handleCharSelectResponse(MessageIn &msg)
{
    int errMsg = msg.readInt8();

    if (errMsg == ERRMSG_OK)
    {
        token = msg.readString(32);
        std::string gameServer = msg.readString();
        unsigned short gameServerPort = msg.readInt16();
        std::string chatServer = msg.readString();
        unsigned short chatServerPort = msg.readInt16();

        logger->log("Game server: %s:%d", gameServer.c_str(), gameServerPort);
        logger->log("Chat server: %s:%d", chatServer.c_str(), chatServerPort);

        gameServerConnection->connect(gameServer, gameServerPort);
        chatServerConnection->connect(chatServer, chatServerPort);

        // Keep the selected character and delete the others
        player_node = mCharInfo->getEntry();
        int slot = mCharInfo->getPos();
        mCharInfo->unlock();
        mCharInfo->select(0);

        do {
            LocalPlayer *tmp = mCharInfo->getEntry();
            if (tmp != player_node)
            {
                delete tmp;
                mCharInfo->setEntry(0);
            }
            mCharInfo->next();
        } while (mCharInfo->getPos());
        mCharInfo->select(slot);

        mCharInfo->clear(); //player_node will be deleted by ~Game

        state = STATE_CONNECT_GAME;
    }
}

LocalPlayer* CharServerHandler::readPlayerData(MessageIn &msg, int &slot)
{
    LocalPlayer *tempPlayer = new LocalPlayer;
    slot = msg.readInt8(); // character slot
    tempPlayer->setName(msg.readString());
    tempPlayer->setGender(msg.readInt8() == GENDER_MALE ? GENDER_MALE : GENDER_FEMALE);
    int hs = msg.readInt8(), hc = msg.readInt8();
    tempPlayer->setHairStyle(hs, hc);
    tempPlayer->setLevel(msg.readInt16());
    tempPlayer->setCharacterPoints(msg.readInt16());
    tempPlayer->setCorrectionPoints(msg.readInt16());
    tempPlayer->setMoney(msg.readInt32());

    for (int i = 0; i < 7; i++)
    {
        tempPlayer->setAttributeBase(i, msg.readInt8());
    }

    return tempPlayer;
}
