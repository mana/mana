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

#include "net/manaserv/charhandler.h"

#include "client.h"
#include "localplayer.h"
#include "log.h"

#include "gui/charcreatedialog.h"
#include "gui/okdialog.h"

#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/net.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/gamehandler.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/protocol.h"

#include "resources/colordb.h"

#include "utils/dtor.h"
#include "utils/gettext.h"

extern Net::CharHandler *charHandler;
extern ManaServ::GameHandler *gameHandler;

namespace ManaServ {

extern Connection *accountServerConnection;
extern Connection *gameServerConnection;
extern Connection *chatServerConnection;
extern std::string netToken;
extern ServerInfo gameServer;
extern ServerInfo chatServer;

CharHandler::CharHandler()
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
            handleCharacterCreateResponse(msg);
            break;

        case APMSG_CHAR_DELETE_RESPONSE:
            handleCharacterDeleteResponse(msg);
            break;

        case APMSG_CHAR_INFO:
            handleCharacterInfo(msg);
            break;

        case APMSG_CHAR_SELECT_RESPONSE:
            handleCharacterSelectResponse(msg);
            break;
    }
}

void CharHandler::handleCharacterInfo(Net::MessageIn &msg)
{
    CachedCharacterInfo info;
    info.slot = msg.readInt8();
    info.name = msg.readString();
    info.gender = msg.readInt8() == GENDER_MALE ? GENDER_MALE :
                                                  GENDER_FEMALE;
    info.hairStyle = msg.readInt8();
    info.hairColor = msg.readInt8();
    info.level = msg.readInt16();
    info.characterPoints = msg.readInt16();
    info.correctionPoints = msg.readInt16();
    info.money = msg.readInt32();

    for (int i = 0; i < 7; i++)
    {
        info.attribute[i] = msg.readInt8();
    }

    mCachedCharacterInfos.push_back(info);

    updateCharacters();
}

void CharHandler::handleCharacterCreateResponse(Net::MessageIn &msg)
{
    const int errMsg = msg.readInt8();

    if (errMsg != ERRMSG_OK)
    {
        // Character creation failed
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
        // Close the character create dialog
        if (mCharCreateDialog)
        {
            mCharCreateDialog->scheduleDelete();
            mCharCreateDialog = 0;
        }
    }
}

void CharHandler::handleCharacterDeleteResponse(Net::MessageIn &msg)
{
    int errMsg = msg.readInt8();
    if (errMsg == ERRMSG_OK)
    {
        // Character deletion successful
        delete mSelectedCharacter;
        mCharacters.remove(mSelectedCharacter);
        updateCharSelectDialog();
        unlockCharSelectDialog();
        new OkDialog(_("Info"), _("Player deleted."));
    }
    else
    {
        // Character deletion failed
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
                errorMessage = strprintf(_("Unknown error (%d)."), errMsg);
        }
        new OkDialog(_("Error"), errorMessage);
    }
    mSelectedCharacter = 0;
}

void CharHandler::handleCharacterSelectResponse(Net::MessageIn &msg)
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

        // Prevent the selected local player from being deleted
        player_node = mSelectedCharacter->dummy;
        mSelectedCharacter->dummy = 0;

        mCachedCharacterInfos.clear();
        updateCharacters();

        Client::setState(STATE_CONNECT_GAME);
    }
    else if (errMsg == ERRMSG_FAILURE)
    {
        errorMessage = _("No gameservers are available.");
        delete_all(mCharacters);
        mCharacters.clear();
        Client::setState(STATE_ERROR);
    }
}

void CharHandler::setCharSelectDialog(CharSelectDialog *window)
{
    mCharSelectDialog = window;
    updateCharacters();
}

void CharHandler::setCharCreateDialog(CharCreateDialog *window)
{
    mCharCreateDialog = window;

    if (!mCharCreateDialog)
        return;

    std::vector<std::string> attributes;
    attributes.push_back(_("Strength:"));
    attributes.push_back(_("Agility:"));
    attributes.push_back(_("Dexterity:"));
    attributes.push_back(_("Vitality:"));
    attributes.push_back(_("Intelligence:"));
    attributes.push_back(_("Willpower:"));

    mCharCreateDialog->setAttributes(attributes, 60, 1, 20);
}

void CharHandler::requestCharacters()
{
    if (!accountServerConnection->isConnected())
    {
        Net::getLoginHandler()->connect();
    }
    else
    {
        // The characters are already there, continue to character selection
        Client::setState(STATE_CHAR_SELECT);
    }
}

void CharHandler::chooseCharacter(Net::Character *character)
{
    mSelectedCharacter = character;

    MessageOut msg(PAMSG_CHAR_SELECT);
    msg.writeInt8(mSelectedCharacter->slot);
    accountServerConnection->send(msg);
}

void CharHandler::newCharacter(const std::string &name,
                               int /* slot */,
                               bool gender,
                               int hairstyle,
                               int hairColor,
                               const std::vector<int> &stats)
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

void CharHandler::deleteCharacter(Net::Character *character)
{
    mSelectedCharacter = character;

    MessageOut msg(PAMSG_CHAR_DELETE);
    msg.writeInt8(mSelectedCharacter->slot);
    accountServerConnection->send(msg);
}

void CharHandler::switchCharacter()
{
    gameHandler->quit(true);
}

int CharHandler::baseSprite() const
{
    return SPRITE_BASE;
}

int CharHandler::hairSprite() const
{
    return SPRITE_HAIR;
}

int CharHandler::maxSprite() const
{
    return SPRITE_VECTOREND;
}

void CharHandler::updateCharacters()
{
    // Delete previous characters
    delete_all(mCharacters);
    mCharacters.clear();

    if (!mCharSelectDialog)
        return;

    // Create new characters and initialize them from the cached infos
    for (unsigned i = 0; i < mCachedCharacterInfos.size(); ++i)
    {
        const CachedCharacterInfo &info = mCachedCharacterInfos.at(i);

        Net::Character *character = new Net::Character;
        character->slot = info.slot;
        LocalPlayer *player = character->dummy;
        player->setName(info.name);
        player->setGender(info.gender);
        player->setSprite(SPRITE_HAIR, info.hairStyle * -1,
                          ColorDB::get(info.hairColor));
        player->setLevel(info.level);
        player->setCharacterPoints(info.characterPoints);
        player->setCorrectionPoints(info.correctionPoints);
        player->setMoney(info.money);

        for (int i = 0; i < 7; i++)
        {
            player->setAttributeBase(i, info.attribute[i], false);
        }

        mCharacters.push_back(character);
    }

    updateCharSelectDialog();
}

} // namespace ManaServ
