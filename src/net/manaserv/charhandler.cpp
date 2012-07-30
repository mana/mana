/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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
#include "net/manaserv/beinghandler.h"
#include "net/manaserv/inventoryhandler.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/manaserv_protocol.h"
#include "net/manaserv/attributes.h"

#include "resources/hairdb.h"

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

CharHandler::~CharHandler()
{
    clear();
}

void CharHandler::handleMessage(MessageIn &msg)
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

void CharHandler::handleCharacterInfo(MessageIn &msg)
{
    CachedCharacterInfo info;
    info.slot = msg.readInt8();
    info.name = msg.readString();
    info.gender = msg.readInt8() == ManaServ::GENDER_MALE ?
                                        ::GENDER_MALE : ::GENDER_FEMALE;
    info.hairStyle = msg.readInt8();
    info.hairColor = msg.readInt8();
    info.level = msg.readInt16();
    info.characterPoints = msg.readInt16();
    info.correctionPoints = msg.readInt16();

    while (msg.getUnreadLength() > 0)
    {
        int id = msg.readInt32();
        CachedAttrbiute attr;
        attr.base = msg.readInt32() / 256.0;
        attr.mod = msg.readInt32() / 256.0;

        info.attribute[id] = attr;
    }

    mCachedCharacterInfos.push_back(info);

    updateCharacters();
}

void CharHandler::handleCharacterCreateResponse(MessageIn &msg)
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
            case CREATE_ATTRIBUTES_TOO_HIGH:
                errorMessage = _("Character's stats are too high.");
                break;
            case CREATE_ATTRIBUTES_TOO_LOW:
                errorMessage = _("Character's stats are too low.");
                break;
            case CREATE_ATTRIBUTES_OUT_OF_RANGE:
                errorMessage = strprintf( _("At least one stat"
                                   "is out of the permitted range: (%u - %u)."),
                                   Attributes::getAttributeMinimum(),
                                   Attributes::getAttributeMaximum());
                break;
            case CREATE_INVALID_SLOT:
                errorMessage = _("Invalid slot number.");
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

void CharHandler::handleCharacterDeleteResponse(MessageIn &msg)
{
    int errMsg = msg.readInt8();
    if (errMsg == ERRMSG_OK)
    {
        // Character deletion successful
        for (unsigned i = 0; i < mCachedCharacterInfos.size(); ++i)
        {
            if (mCachedCharacterInfos[i].slot == mSelectedCharacter->slot)
            {
                mCachedCharacterInfos.erase(mCachedCharacterInfos.begin() + i);
                break;
            }
        }
        delete mSelectedCharacter;
        mCharacters.remove(mSelectedCharacter);
        updateCharSelectDialog();
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
    unlockCharSelectDialog();
}

void CharHandler::handleCharacterSelectResponse(MessageIn &msg)
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

        // Prevent the selected local player from being deleted
        local_player = mSelectedCharacter->dummy;
        PlayerInfo::setBackend(mSelectedCharacter->data);
        mSelectedCharacter->dummy = 0;

        Client::setState(STATE_CONNECT_GAME);
    }
    else
    {
        switch (errMsg)
        {
            case ERRMSG_FAILURE:
                errorMessage = _("No gameservers are available.");
                break;
            case ERRMSG_INVALID_ARGUMENT:
                errorMessage = _("Invalid character slot selected.");
                break;
            default:
                errorMessage = strprintf(_("Unhandled character select "
                                           "error message %i."), errMsg);
        }
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
                               int slot,
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
    msg.writeInt8(slot);

    std::vector<int>::const_iterator it, it_end;
    for (it = stats.begin(), it_end = stats.end(); it != it_end; it++)
        msg.writeInt16((*it));

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

unsigned int CharHandler::baseSprite() const
{
    return SPRITE_LAYER_BASE;
}

unsigned int CharHandler::hairSprite() const
{
    return SPRITE_LAYER_HAIR;
}

unsigned int CharHandler::maxSprite() const
{
    static unsigned int visibleSlots = FIXED_SPRITE_LAYER_SIZE
        + Net::getInventoryHandler()->getVisibleSlotsNumber();
    return visibleSlots;
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
        LocalPlayer *player = character->dummy = new LocalPlayer;
        player->setName(info.name);
        player->setGender(info.gender);
        player->setSprite(SPRITE_LAYER_HAIR, info.hairStyle * -1,
                          hairDB.getHairColor(info.hairColor));
        character->data.mAttributes[LEVEL] = info.level;
        character->data.mAttributes[CHAR_POINTS] = info.characterPoints;
        character->data.mAttributes[CORR_POINTS] = info.correctionPoints;

        for (CachedAttributes::const_iterator it = info.attribute.begin(),
             it_end = info.attribute.end(); it != it_end; it++)
        {
            character->data.mStats[i].base = it->second.base;
            character->data.mStats[i].mod = it->second.mod;
        }

        mCharacters.push_back(character);
    }

    updateCharSelectDialog();
}

void CharHandler::clear()
{
    setCharCreateDialog(0);
    setCharSelectDialog(0);

    mCachedCharacterInfos.clear();
    updateCharacters();
}

} // namespace ManaServ
