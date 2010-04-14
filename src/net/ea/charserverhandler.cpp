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

#include "net/ea/charserverhandler.h"

#include "client.h"
#include "game.h"
#include "log.h"

#include "gui/charcreatedialog.h"
#include "gui/okdialog.h"

#include "net/logindata.h"
#include "net/messagein.h"
#include "net/messageout.h"
#include "net/net.h"

#include "net/ea/gamehandler.h"
#include "net/ea/loginhandler.h"
#include "net/ea/network.h"
#include "net/ea/protocol.h"

#include "resources/colordb.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

extern Net::CharHandler *charHandler;

namespace EAthena {

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

void CharServerHandler::handleMessage(Net::MessageIn &msg)
{
    logger->log("CharServerHandler: Packet ID: %x, Length: %d",
                msg.getId(), msg.getLength());

    switch (msg.getId())
    {
            case SMSG_CHAR_LOGIN:
            {
                msg.skip(2);  // Length word
                msg.skip(20); // Unused

                // Derive number of characters from message length
                const int count = (msg.getLength() - 24) / 106;

                for (int i = 0; i < count; ++i)
                {
                    Net::Character *character = new Net::Character;
                    int slot;
                    character->dummy = readPlayerData(msg, &slot);
                    character->slot = slot;
                    mCharacters.push_back(character);
                    logger->log("CharServer: Player: %s (%d)",
                                character->dummy->getName().c_str(), slot);
                }

                Client::setState(STATE_CHAR_SELECT);
            }
            break;

        case SMSG_CHAR_LOGIN_ERROR:
            switch (msg.readInt8())
            {
                case 0:
                    errorMessage = _("Access denied.");
                    break;
                case 1:
                    errorMessage = _("Cannot use this ID.");
                    break;
                default:
                    errorMessage = _("Unknown failure to select character.");
                    break;
            }
            unlockCharSelectDialog();
            break;

        case SMSG_CHAR_CREATE_SUCCEEDED:
            {
                Net::Character *character = new Net::Character;
                int slot;
                character->dummy = readPlayerData(msg, &slot);
                character->slot = slot;
                mCharacters.push_back(character);

                updateCharSelectDialog();

                // Close the character create dialog
                if (mCharCreateDialog)
                {
                    mCharCreateDialog->scheduleDelete();
                    mCharCreateDialog = 0;
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
            mSelectedCharacter = 0;
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
            msg.skip(4); // CharID, must be the same as player_node->charID
            GameHandler *gh = static_cast<GameHandler*>(Net::getGameHandler());
            gh->setMap(msg.readString(16));
            mapServer.hostname = ipToString(msg.readInt32());
            mapServer.port = msg.readInt16();

            // Prevent the selected local player from being deleted
            player_node = mSelectedCharacter->dummy;
            mSelectedCharacter->dummy = 0;

            delete_all(mCharacters);
            mCharacters.clear();
            updateCharSelectDialog();

            mNetwork->disconnect();
            Client::setState(STATE_CONNECT_GAME);
        }
        break;

        case SMSG_CHANGE_MAP_SERVER:
        {
            GameHandler *gh = static_cast<GameHandler*>(Net::getGameHandler());
            gh->setMap(msg.readString(16));
            int x = msg.readInt16();
            int y = msg.readInt16();
            mapServer.hostname = ipToString(msg.readInt32());
            mapServer.port = msg.readInt16();

            mNetwork->disconnect();
            Client::setState(STATE_CHANGE_MAP);
            player_node->setTileCoords(x, y);
            player_node->setMap(0);
        }
        break;
    }
}

LocalPlayer *CharServerHandler::readPlayerData(Net::MessageIn &msg, int *slot)
{
    const Token &token =
            static_cast<LoginHandler*>(Net::getLoginHandler())->getToken();

    LocalPlayer *tempPlayer = new LocalPlayer(msg.readInt32(), 0);
    tempPlayer->setGender(token.sex);

    tempPlayer->setExp(msg.readInt32());
    tempPlayer->setMoney(msg.readInt32());
    tempPlayer->setExperience(JOB, msg.readInt32(), 1);
    int temp = msg.readInt32();
    tempPlayer->setAttributeBase(JOB, temp, false);
    tempPlayer->setAttributeEffective(JOB, temp);
    tempPlayer->setSprite(SPRITE_SHOE, msg.readInt16());
    tempPlayer->setSprite(SPRITE_GLOVES, msg.readInt16());
    tempPlayer->setSprite(SPRITE_CAPE, msg.readInt16());
    tempPlayer->setSprite(SPRITE_MISC1, msg.readInt16());
    msg.readInt32();                       // option
    msg.readInt32();                       // karma
    msg.readInt32();                       // manner
    msg.skip(2);                          // unknown
    tempPlayer->setHp(msg.readInt16());
    tempPlayer->setMaxHp(msg.readInt16());
    tempPlayer->setMP(msg.readInt16());
    tempPlayer->setMaxMP(msg.readInt16());
    msg.readInt16();                       // speed
    tempPlayer->setSubtype(msg.readInt16()); // class (used for race)
    int hairStyle = msg.readInt16();
    Uint16 weapon = msg.readInt16();
    tempPlayer->setSprite(SPRITE_WEAPON, weapon, "", true);
    tempPlayer->setLevel(msg.readInt16());
    msg.readInt16();                       // skill point
    tempPlayer->setSprite(SPRITE_BOTTOMCLOTHES, msg.readInt16()); // head bottom
    tempPlayer->setSprite(SPRITE_SHIELD, msg.readInt16());
    tempPlayer->setSprite(SPRITE_HAT, msg.readInt16()); // head option top
    tempPlayer->setSprite(SPRITE_TOPCLOTHES, msg.readInt16()); // head option mid
    tempPlayer->setSprite(SPRITE_HAIR, hairStyle * -1, ColorDB::get(msg.readInt16()));
    tempPlayer->setSprite(SPRITE_MISC2, msg.readInt16());
    tempPlayer->setName(msg.readString(24));
    for (int i = 0; i < 6; i++)
        tempPlayer->setAttributeBase(i + STR, msg.readInt8(), false);
    *slot = msg.readInt8(); // character slot
    msg.readInt8();                        // unknown

    return tempPlayer;
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
    attributes.push_back(_("Strength:"));
    attributes.push_back(_("Agility:"));
    attributes.push_back(_("Vitality:"));
    attributes.push_back(_("Intelligence:"));
    attributes.push_back(_("Dexterity:"));
    attributes.push_back(_("Luck:"));

    const Token &token =
            static_cast<LoginHandler*>(Net::getLoginHandler())->getToken();

    mCharCreateDialog->setAttributes(attributes, 30, 1, 9);
    mCharCreateDialog->setFixedGender(true, token.sex);
}

void CharServerHandler::requestCharacters()
{
    connect();
}

void CharServerHandler::chooseCharacter(Net::Character *character)
{
    mSelectedCharacter = character;

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
    MessageOut outMsg(CMSG_PLAYER_RESTART);
    outMsg.writeInt8(1);
}

int CharServerHandler::baseSprite() const
{
    return SPRITE_BASE;
}

int CharServerHandler::hairSprite() const
{
    return SPRITE_HAIR;
}

int CharServerHandler::maxSprite() const
{
    return SPRITE_VECTOREND;
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
    outMsg.writeInt8((token.sex == GENDER_MALE) ? 1 : 0);

    // We get 4 useless bytes before the real answer comes in (what are these?)
    mNetwork->skip(4);
}

} // namespace EAthena
