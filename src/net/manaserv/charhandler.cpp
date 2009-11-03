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

#include "net/manaserv/charhandler.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/protocol.h"

#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/net.h"

#include "game.h"
#include "localplayer.h"
#include "log.h"
#include "main.h"

#include "gui/charcreatedialog.h"
#include "gui/okdialog.h"

#include "resources/colordb.h"

#include "utils/gettext.h"

Net::CharHandler *charHandler;

struct CharInfo {
    unsigned char slot;
    std::string name;
    Gender gender;
    int hs, hc;
    unsigned short level;
    unsigned short charPoints;
    unsigned short corrPoints;
    unsigned int money;
    unsigned char attr[7];
};

typedef std::vector<CharInfo> CharInfos;
CharInfos chars;

namespace ManaServ {

extern Connection *accountServerConnection;
extern Connection *gameServerConnection;
extern Connection *chatServerConnection;
extern std::string netToken;
extern ServerInfo gameServer;
extern ServerInfo chatServer;

CharHandler::CharHandler():
    mCharSelectDialog(0),
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
    charHandler = this;
}

void CharHandler::handleMessage(Net::MessageIn &msg)
{
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
                LocalPlayer *tempPlayer = mCharInfo->getEntry();
                mCharInfo->setEntry(0);
                mCharInfo->unlock();
                if (mCharSelectDialog)
                    mCharSelectDialog->update(mCharInfo->getPos());
                new OkDialog(_("Info"), _("Player deleted."));
                delete tempPlayer;
            }
            // Character deletion failed
            else
            {
                std::string errorMessage = "";
                switch (errMsg)
                {
                    case ERRMSG_NO_LOGIN:
                        errorMessage = _("Not logged in.");
                        break;
                    case ERRMSG_INVALID_ARGUMENT:
                        errorMessage = _("Selection out of range.");
                        break;
                    default:
                        errorMessage = _("Unknown error.");
                }
                mCharInfo->unlock();
                new OkDialog(_("Error"), errorMessage);
            }
        }
            break;

        case APMSG_CHAR_INFO:
        {
            CharInfo info;
            info.slot = msg.readInt8(); // character slot
            info.name = msg.readString();
            info.gender = msg.readInt8() == GENDER_MALE ? GENDER_MALE :
                                                          GENDER_FEMALE;
            info.hs = msg.readInt8();
            info.hc = msg.readInt8();
            info.level = msg.readInt16();
            info.charPoints = msg.readInt16();
            info.corrPoints = msg.readInt16();
            info.money = msg.readInt32();

            for (int i = 0; i < 7; i++)
            {
                info.attr[i] = msg.readInt8();
            }

            chars.push_back(info);

            if (mCharSelectDialog)
            {
                mCharInfo->select(info.slot);

                LocalPlayer *tempPlayer = new LocalPlayer();
                tempPlayer->setName(info.name);
                tempPlayer->setGender(info.gender);
                tempPlayer->setSprite(Player::HAIR_SPRITE, info.hs * -1,
                                      ColorDB::get(info.hc));
                tempPlayer->setLevel(info.level);
                tempPlayer->setCharacterPoints(info.charPoints);
                tempPlayer->setCorrectionPoints(info.corrPoints);
                tempPlayer->setMoney(info.money);

                for (int i = 0; i < 7; i++)
                {
                    tempPlayer->setAttributeBase(i, info.attr[i]);
                }

                mCharInfo->setEntry(tempPlayer);

                mCharSelectDialog->update(info.slot);
            }
        }
            break;

        case APMSG_CHAR_SELECT_RESPONSE:
            handleCharSelectResponse(msg);
            break;
    }
}

void CharHandler::handleCharCreateResponse(Net::MessageIn &msg)
{
    int errMsg = msg.readInt8();

    // Character creation failed
    if (errMsg != ERRMSG_OK)
    {
        std::string errorMessage = "";
        switch (errMsg)
        {
            case ERRMSG_NO_LOGIN:
                errorMessage = _("Not logged in.");
                break;
            case CREATE_TOO_MUCH_CHARACTERS:
                errorMessage = _("No empty slot.");
                break;
            case ERRMSG_INVALID_ARGUMENT:
                errorMessage = _("Invalid name.");
                break;
            case CREATE_EXISTS_NAME:
                errorMessage = _("Character's name already exists.");
                break;
            case CREATE_INVALID_HAIRSTYLE:
                errorMessage = _("Invalid hairstyle.");
                break;
            case CREATE_INVALID_HAIRCOLOR:
                errorMessage = _("Invalid hair color.");
                break;
            case CREATE_INVALID_GENDER:
                errorMessage = _("Invalid gender.");
                break;
            case CREATE_RAW_STATS_TOO_HIGH:
                errorMessage = _("Character's stats are too high.");
                break;
            case CREATE_RAW_STATS_TOO_LOW:
                errorMessage = _("Character's stats are too low.");
                break;
            case CREATE_RAW_STATS_EQUAL_TO_ZERO:
                errorMessage = _("One stat is zero.");
                break;
            default:
                errorMessage = _("Unknown error.");
                break;
        }
        new OkDialog(_("Error"), errorMessage);

        if (mCharCreateDialog)
            mCharCreateDialog->unlock();
    }
    else
    {
        if (mCharCreateDialog)
        {
            mCharCreateDialog->success();
            mCharCreateDialog->scheduleDelete();
            mCharCreateDialog = 0;
        }
    }

}

void CharHandler::handleCharSelectResponse(Net::MessageIn &msg)
{
    int errMsg = msg.readInt8();

    if (errMsg == ERRMSG_OK)
    {
        netToken = msg.readString(32);

        gameServer.hostname.assign(msg.readString());
        gameServer.port = msg.readInt16();

        chatServer.hostname.assign(msg.readString());
        chatServer.port = msg.readInt16();

        logger->log("Game server: %s:%d", gameServer.hostname.c_str(),
                    gameServer.port);
        logger->log("Chat server: %s:%d", chatServer.hostname.c_str(),
                    chatServer.port);

        gameServerConnection->connect(gameServer.hostname, gameServer.port);
        chatServerConnection->connect(chatServer.hostname, chatServer.port);

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
    else if(errMsg == ERRMSG_FAILURE)
    {
        errorMessage = _("No gameservers are available.");
        mCharInfo->clear();
        state = STATE_ERROR;
    }
}

void CharHandler::setCharSelectDialog(CharSelectDialog *window)
{
    mCharSelectDialog = window;
}

void CharHandler::setCharCreateDialog(CharCreateDialog *window)
{
    mCharCreateDialog = window;

    if (!mCharCreateDialog) return;

    std::vector<std::string> attributes;
    attributes.push_back(_("Strength:"));
    attributes.push_back(_("Agility:"));
    attributes.push_back(_("Dexterity:"));
    attributes.push_back(_("Vitality:"));
    attributes.push_back(_("Intelligence:"));
    attributes.push_back(_("Willpower:"));

    mCharCreateDialog->setAttributes(attributes, 60, 1, 20);
}

void CharHandler::getCharacters()
{
    if (!accountServerConnection->isConnected())
        Net::getLoginHandler()->connect();
    else
    {
        mCharInfo->unlock();
        LocalPlayer *tempPlayer;
        for (CharInfos::const_iterator it = chars.begin(); it != chars.end(); it++)
        {
            const CharInfo info = (CharInfo) (*it);
            mCharInfo->select(info.slot);

            tempPlayer = new LocalPlayer();
            tempPlayer->setName(info.name);
            tempPlayer->setGender(info.gender);
            tempPlayer->setSprite(Player::HAIR_SPRITE, info.hs * -1,
                                  ColorDB::get(info.hc));
            tempPlayer->setLevel(info.level);
            tempPlayer->setCharacterPoints(info.charPoints);
            tempPlayer->setCorrectionPoints(info.corrPoints);
            tempPlayer->setMoney(info.money);

            for (int i = 0; i < 7; i++)
            {
                tempPlayer->setAttributeBase(i, info.attr[i]);
            }

            mCharInfo->setEntry(tempPlayer);
        }

        // Close the character create dialog
        if (mCharCreateDialog)
        {
            mCharCreateDialog->scheduleDelete();
            mCharCreateDialog = 0;
        }

        state = STATE_CHAR_SELECT;
    }
}

void CharHandler::chooseCharacter(int slot, LocalPlayer* character)
{
    MessageOut msg(PAMSG_CHAR_SELECT);

    msg.writeInt8(slot);

    accountServerConnection->send(msg);
}

void CharHandler::newCharacter(const std::string &name, int slot, bool gender,
                  int hairstyle, int hairColor, std::vector<int> stats)
{
    MessageOut msg(PAMSG_CHAR_CREATE);

    msg.writeString(name);
    msg.writeInt8(hairstyle);
    msg.writeInt8(hairColor);
    msg.writeInt8(gender);
    msg.writeInt16(stats[0]);
    msg.writeInt16(stats[1]);
    msg.writeInt16(stats[2]);
    msg.writeInt16(stats[3]);
    msg.writeInt16(stats[4]);
    msg.writeInt16(stats[5]);

    accountServerConnection->send(msg);
}

void CharHandler::deleteCharacter(int slot, LocalPlayer* character)
{
    MessageOut msg(PAMSG_CHAR_DELETE);

    msg.writeInt8(slot);

    accountServerConnection->send(msg);
}

void CharHandler::switchCharacter()
{
    // TODO
}

} // namespace ManaServ
