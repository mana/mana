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

#include "net/tmwa/charserverhandler.h"

#include "client.h"
#include "game.h"
#include "log.h"

#include "gui/charcreatedialog.h"
#include "gui/okdialog.h"

#include "net/net.h"

#include "net/tmwa/gamehandler.h"
#include "net/tmwa/loginhandler.h"
#include "net/tmwa/messagein.h"
#include "net/tmwa/messageout.h"
#include "net/tmwa/network.h"
#include "net/tmwa/protocol.h"

#include "resources/attributes.h"
#include "resources/chardb.h"
#include "resources/hairdb.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

extern Net::CharHandler *charHandler;

namespace TmwAthena {

extern ServerInfo charServer;
extern ServerInfo mapServer;

CharServerHandler::CharServerHandler()
{
    static const Uint16 _messages[] = {
        SMSG_CHAR_LOGIN,
        SMSG_CHAR_LOGIN_ERROR,
        SMSG_CHAR_CREATE_SUCCEEDED,
        SMSG_CHAR_CREATE_FAILED,
        SMSG_CHAR_DELETE_SUCCEEDED,
        SMSG_CHAR_DELETE_FAILED,
        SMSG_CHAR_MAP_INFO,
        SMSG_CHANGE_MAP_SERVER,
        0
    };
    handledMessages = _messages;
    charHandler = this;
}

void CharServerHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_CHAR_LOGIN:
            {
                msg.skip(2);  // Length word
                msg.skip(20); // Unused

                delete_all(mCharacters);
                mCharacters.clear();

                // Derive number of characters from message length
                const int count = (msg.getLength() - 24) / 106;

                for (int i = 0; i < count; ++i)
                {
                    auto *character = new Net::Character;
                    readPlayerData(msg, character);
                    mCharacters.push_back(character);
                    Log::info("CharServer: Player: %s (%d)",
                              character->dummy->getName().c_str(), character->slot);
                }

                Client::setState(STATE_CHAR_SELECT);
            }
            break;

        case SMSG_CHAR_LOGIN_ERROR:
            switch (msg.readInt8())
            {
                case 0:
                    errorMessage = _("Access denied. Most likely, there are "
                                     "too many players on this server.");
                    break;
                case 1:
                    errorMessage = _("Cannot use this ID.");
                    break;
                default:
                    errorMessage = _("Unknown char-server failure.");
                    break;
            }
            Client::setState(STATE_ERROR);
            break;

        case SMSG_CHAR_CREATE_SUCCEEDED:
            {
                auto *character = new Net::Character;
                readPlayerData(msg, character);
                mCharacters.push_back(character);

                updateCharSelectDialog();

                // Close the character create dialog
                if (mCharCreateDialog)
                {
                    mCharCreateDialog->scheduleDelete();
                    mCharCreateDialog = nullptr;
                }
            }
            break;

        case SMSG_CHAR_CREATE_FAILED:
            new OkDialog(_("Error"), _("Failed to create character. Most "
                                       "likely the name is already taken."));
            if (mCharCreateDialog)
                mCharCreateDialog->unlock();
            break;

        case SMSG_CHAR_DELETE_SUCCEEDED:
            delete mSelectedCharacter;
            mCharacters.remove(mSelectedCharacter);
            mSelectedCharacter = nullptr;
            updateCharSelectDialog();
            unlockCharSelectDialog();
            new OkDialog(_("Info"), _("Character deleted."));
            break;

        case SMSG_CHAR_DELETE_FAILED:
            unlockCharSelectDialog();
            new OkDialog(_("Error"), _("Failed to delete character."));
            break;

        case SMSG_CHAR_MAP_INFO:
        {
            msg.skip(4); // CharID, must be the same as local_player->charID
            auto *gh = static_cast<GameHandler*>(Net::getGameHandler());
            gh->setMap(msg.readString(16));

            const auto ip = msg.readInt32();

            if (charServer.persistentIp)
                mapServer.hostname = charServer.hostname;
            else
                mapServer.hostname = ipToString(ip);

            mapServer.port = msg.readInt16();

            local_player = mSelectedCharacter->dummy;
            PlayerInfo::setBackend(mSelectedCharacter->data);

            // Prevent the selected local player from being deleted
            mSelectedCharacter->dummy = nullptr;

            delete_all(mCharacters);
            mCharacters.clear();
            updateCharSelectDialog();

            mNetwork->disconnect();
            Client::setState(STATE_CONNECT_GAME);
        }
        break;

        case SMSG_CHANGE_MAP_SERVER:
        {
            auto *gh = static_cast<GameHandler*>(Net::getGameHandler());
            gh->setMap(msg.readString(16));
            int x = msg.readInt16();
            int y = msg.readInt16();
            mapServer.hostname = ipToString(msg.readInt32());
            mapServer.port = msg.readInt16();

            mNetwork->disconnect();
            Client::setState(STATE_CHANGE_MAP);
            Map *map = local_player->getMap();
            const int tileWidth = map->getTileWidth();
            const int tileHeight = map->getTileHeight();
            local_player->setPosition(Vector(x * tileWidth + tileWidth / 2,
                                            y * tileHeight + tileHeight / 2));
            local_player->setMap(nullptr);
        }
        break;
    }
}

void CharServerHandler::readPlayerData(MessageIn &msg, Net::Character *character)
{
    const Token &token =
            static_cast<LoginHandler*>(Net::getLoginHandler())->getToken();

    const int id = msg.readInt32();

    character->data.mAttributes[EXP] = msg.readInt32();
    character->data.mAttributes[MONEY] = msg.readInt32();
    character->data.mStats[JOB].exp = msg.readInt32();

    const int temp = msg.readInt32();
    character->data.mStats[JOB].base = temp;
    character->data.mStats[JOB].mod = temp;

    const int shoe = msg.readInt16();
    const int gloves = msg.readInt16();
    const int cape = msg.readInt16();
    const int misc1 = msg.readInt16();

    msg.readInt32();                       // option
    msg.readInt32();                       // karma
    msg.readInt32();                       // manner
    msg.readInt16();                       // character points left

    character->data.mAttributes[HP] = msg.readInt16();
    character->data.mAttributes[MAX_HP] = msg.readInt16();
    character->data.mAttributes[MP] = msg.readInt16();
    character->data.mAttributes[MAX_MP] = msg.readInt16();

    msg.readInt16();                       // speed
    const uint16_t race = msg.readInt16(); // class (used for race)
    int hairStyle = msg.readInt8();
    msg.readInt8();                        // look
    const uint16_t weapon = msg.readInt16();

    auto *tempPlayer = new LocalPlayer(id, race);
    tempPlayer->setGender(token.sex);

    tempPlayer->setSprite(SPRITE_SHOE, shoe);
    tempPlayer->setSprite(SPRITE_GLOVES, gloves);
    tempPlayer->setSprite(SPRITE_CAPE, cape);
    tempPlayer->setSprite(SPRITE_MISC1, misc1);
    tempPlayer->setSprite(SPRITE_WEAPON, weapon, "", true);

    character->data.mAttributes[LEVEL] = msg.readInt16();

    msg.readInt16();                       // skill point
    tempPlayer->setSprite(SPRITE_BOTTOMCLOTHES, msg.readInt16()); // head bottom
    tempPlayer->setSprite(SPRITE_SHIELD, msg.readInt16());
    tempPlayer->setSprite(SPRITE_HAT, msg.readInt16()); // head option top
    tempPlayer->setSprite(SPRITE_TOPCLOTHES, msg.readInt16()); // head option mid
    tempPlayer->setSprite(SPRITE_HAIR, hairStyle * -1,
                          hairDB.getHairColor(msg.readInt16()));
    tempPlayer->setSprite(SPRITE_MISC2, msg.readInt16());
    tempPlayer->setName(msg.readString(24));

    character->dummy = tempPlayer;

    for (int i = 0; i < 6; i++)
        character->data.mStats[i + STRENGTH].base = msg.readInt8();

    character->slot = msg.readInt8(); // character slot
    const uint8_t sex = msg.readInt8();
    tempPlayer->setGender(sex ? Gender::Male : Gender::Female);
}

void CharServerHandler::setCharSelectDialog(CharSelectDialog *window)
{
    mCharSelectDialog = window;
    updateCharSelectDialog();
}

void CharServerHandler::setCharCreateDialog(CharCreateDialog *window)
{
    mCharCreateDialog = window;

    if (!mCharCreateDialog)
        return;

    std::vector<std::string> attributes;
    attributes.emplace_back(_("Strength:"));
    attributes.emplace_back(_("Agility:"));
    attributes.emplace_back(_("Vitality:"));
    attributes.emplace_back(_("Intelligence:"));
    attributes.emplace_back(_("Dexterity:"));
    attributes.emplace_back(_("Luck:"));

    const Token &token =
            static_cast<LoginHandler*>(Net::getLoginHandler())->getToken();

    unsigned minStat = CharDB::getMinStat();
    if (minStat == 0)
        minStat = Attributes::getAttributeMinimum();
    unsigned maxStat = CharDB::getMaxStat();
    if (maxStat == 0)
        maxStat = Attributes::getAttributeMaximum();
    unsigned sumStat = CharDB::getSumStat();
    if (sumStat == 0)
        sumStat = Attributes::getCreationPoints();

    mCharCreateDialog->setAttributes(attributes, sumStat, minStat, maxStat);
    mCharCreateDialog->setDefaultGender(token.sex);
}

void CharServerHandler::requestCharacters()
{
    connect();
}

void CharServerHandler::chooseCharacter(Net::Character *character)
{
    mSelectedCharacter = character;
    mCharSelectDialog = nullptr;

    MessageOut outMsg(CMSG_CHAR_SELECT);
    outMsg.writeInt8(mSelectedCharacter->slot);
}

void CharServerHandler::newCharacter(const std::string &name, int slot,
                                     bool gender, int hairstyle, int hairColor,
                                     const std::vector<int> &stats)
{
    MessageOut outMsg(CMSG_CHAR_CREATE);
    outMsg.writeString(name, 24);
    for (int i = 0; i < 6; i++)
    {
        outMsg.writeInt8(stats[i]);
    }
    outMsg.writeInt8(slot);
    outMsg.writeInt16(hairColor);
    outMsg.writeInt16(hairstyle);
}

void CharServerHandler::deleteCharacter(Net::Character *character)
{
    mSelectedCharacter = character;

    MessageOut outMsg(CMSG_CHAR_DELETE);
    outMsg.writeInt32(mSelectedCharacter->dummy->getId());
    outMsg.writeString("a@a.com", 40);
}

void CharServerHandler::switchCharacter()
{
    // This is really a map-server packet
    MessageOut outMsg(CMSG_PLAYER_REBOOT);
    outMsg.writeInt8(1);
}

unsigned int CharServerHandler::baseSprite() const
{
    return SPRITE_BASE;
}

unsigned int CharServerHandler::hairSprite() const
{
    return SPRITE_HAIR;
}

unsigned int CharServerHandler::maxSprite() const
{
    return SPRITE_VECTOREND;
}

int CharServerHandler::getCharCreateMinHairColorId() const
{
    return CharDB::getMinHairColor();
}

int CharServerHandler::getCharCreateMaxHairColorId() const
{
    const int max = CharDB::getMaxHairColor();
    return max ? max : 11; // default maximum
}

int CharServerHandler::getCharCreateMaxHairStyleId() const
{
    const int max = CharDB::getMaxHairStyle();
    return max ? max : 19; // default maximum
}

void CharServerHandler::connect()
{
    const Token &token =
            static_cast<LoginHandler*>(Net::getLoginHandler())->getToken();

    mNetwork->disconnect();
    mNetwork->connect(charServer);
    MessageOut outMsg(CMSG_CHAR_SERVER_CONNECT);
    outMsg.writeInt32(token.account_ID);
    outMsg.writeInt32(token.session_ID1);
    outMsg.writeInt32(token.session_ID2);
    // [Fate] The next word is unused by the old char server, so we squeeze in
    //        mana client version information
    outMsg.writeInt16(CLIENT_PROTOCOL_VERSION);
    outMsg.writeInt8(token.sex == Gender::Male ? 1 : 0);

    // We get 4 useless bytes before the real answer comes in (what are these?)
    mNetwork->skip(4);
}

} // namespace TmwAthena
