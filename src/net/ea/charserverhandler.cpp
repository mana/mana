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

#include "net/ea/charserverhandler.h"

#include "net/ea/loginhandler.h"
#include "net/ea/network.h"
#include "net/ea/protocol.h"

#include "net/logindata.h"
#include "net/messagein.h"
#include "net/messageout.h"
#include "net/net.h"

#include "game.h"
#include "log.h"
#include "main.h"

#include "gui/charcreatedialog.h"
#include "gui/okdialog.h"

#include "resources/colordb.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

Net::CharHandler *charHandler;

namespace EAthena {

extern ServerInfo charServer;
extern ServerInfo mapServer;

CharServerHandler::CharServerHandler():
    mCharSelectDialog(0),
    mCharCreateDialog(0)
{
    static const Uint16 _messages[] = {
        SMSG_CHAR_LOGIN,
        SMSG_CHAR_LOGIN_ERROR,
        SMSG_CHAR_CREATE_SUCCEEDED,
        SMSG_CHAR_CREATE_FAILED,
        SMSG_CHAR_DELETE_SUCCEEDED,
        SMSG_CHAR_DELETE_FAILED,
        SMSG_CHAR_MAP_INFO,
        0
    };
    handledMessages = _messages;
    charHandler = this;
}

void CharServerHandler::handleMessage(Net::MessageIn &msg)
{
    int count, slot;
    LocalPlayer *tempPlayer;

    logger->log("CharServerHandler: Packet ID: %x, Length: %d",
            msg.getId(), msg.getLength());
    switch (msg.getId())
    {
        case SMSG_CHAR_LOGIN:
            msg.skip(2); // Length word
            msg.skip(20); // Unused

            // Derive number of characters from message length
            count = (msg.getLength() - 24) / 106;

            for (int i = 0; i < count; i++)
            {
                tempPlayer = readPlayerData(msg, slot);
                mCharInfo->select(slot);
                mCharInfo->setEntry(tempPlayer);
                logger->log("CharServer: Player: %s (%d)",
                tempPlayer->getName().c_str(), slot);
            }

            state = STATE_CHAR_SELECT;
            break;

        case SMSG_CHAR_LOGIN_ERROR:
            switch (msg.readInt8()) {
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
            mCharInfo->unlock();
            break;

        case SMSG_CHAR_CREATE_SUCCEEDED:
            tempPlayer = readPlayerData(msg, slot);
            mCharInfo->unlock();
            mCharInfo->select(slot);
            mCharInfo->setEntry(tempPlayer);

            // Close the character create dialog
            if (mCharCreateDialog)
            {
                mCharCreateDialog->success();
                mCharCreateDialog->scheduleDelete();
                mCharCreateDialog = 0;
            }
            break;

        case SMSG_CHAR_CREATE_FAILED:
            new OkDialog(_("Error"), _("Failed to create character. Most "
                                       "likely the name is already taken."));

            if (mCharCreateDialog)
                mCharCreateDialog->unlock();
            break;

        case SMSG_CHAR_DELETE_SUCCEEDED:
            tempPlayer = mCharInfo->getEntry();
            mCharInfo->setEntry(0);
            mCharInfo->unlock();
            if (mCharSelectDialog)
                mCharSelectDialog->update(mCharInfo->getPos());
            new OkDialog(_("Info"), _("Character deleted."));
            delete tempPlayer;
            break;

        case SMSG_CHAR_DELETE_FAILED:
            mCharInfo->unlock();
            new OkDialog(_("Error"), _("Failed to delete character."));
            break;

        case SMSG_CHAR_MAP_INFO:
            player_node = mCharInfo->getEntry();
            slot = mCharInfo->getPos();
            msg.skip(4); // CharID, must be the same as player_node->charID
            map_path = msg.readString(16);
            mapServer.hostname = ipToString(msg.readInt32());
            mapServer.port = msg.readInt16();
            mCharInfo->unlock();
            mCharInfo->select(0);
            // Clear unselected players infos
            do
            {
                LocalPlayer *tmp = mCharInfo->getEntry();
                if (tmp != player_node)
                {
                    delete tmp;
                    mCharInfo->setEntry(0);
                }
                mCharInfo->next();
            } while (mCharInfo->getPos());

            mCharInfo->select(slot);
            mNetwork->disconnect();
            state = STATE_CONNECT_GAME;
            break;
    }
}

LocalPlayer *CharServerHandler::readPlayerData(Net::MessageIn &msg, int &slot)
{
    const Token &token =
            static_cast<LoginHandler*>(Net::getLoginHandler())->getToken();

    LocalPlayer *tempPlayer = new LocalPlayer(msg.readInt32(), 0, NULL);
    tempPlayer->setGender(token.sex);

    tempPlayer->setExp(msg.readInt32());
    tempPlayer->setMoney(msg.readInt32());
    tempPlayer->setExperience(JOB, msg.readInt32(), 1);
    int temp = msg.readInt32();
    tempPlayer->setAttributeBase(JOB, temp);
    tempPlayer->setAttributeEffective(JOB, temp);
    tempPlayer->setSprite(Player::SHOE_SPRITE, msg.readInt16());
    tempPlayer->setSprite(Player::GLOVES_SPRITE, msg.readInt16());
    tempPlayer->setSprite(Player::CAPE_SPRITE, msg.readInt16());
    tempPlayer->setSprite(Player::MISC1_SPRITE, msg.readInt16());
    msg.readInt32();                       // option
    msg.readInt32();                       // karma
    msg.readInt32();                       // manner
    msg.skip(2);                          // unknown
    tempPlayer->setHp(msg.readInt16());
    tempPlayer->setMaxHp(msg.readInt16());
    tempPlayer->setMP(msg.readInt16());
    tempPlayer->setMaxMP(msg.readInt16());
    msg.readInt16();                       // speed
    msg.readInt16();                       // class
    int hairStyle = msg.readInt16();
    Uint16 weapon = msg.readInt16();
    tempPlayer->setSprite(Player::WEAPON_SPRITE, weapon);
    tempPlayer->setLevel(msg.readInt16());
    msg.readInt16();                       // skill point
    tempPlayer->setSprite(Player::BOTTOMCLOTHES_SPRITE, msg.readInt16()); // head bottom
    tempPlayer->setSprite(Player::SHIELD_SPRITE, msg.readInt16());
    tempPlayer->setSprite(Player::HAT_SPRITE, msg.readInt16()); // head option top
    tempPlayer->setSprite(Player::TOPCLOTHES_SPRITE, msg.readInt16()); // head option mid
    tempPlayer->setSprite(Player::HAIR_SPRITE, hairStyle * -1, ColorDB::get(msg.readInt16()));
    tempPlayer->setSprite(Player::MISC2_SPRITE, msg.readInt16());
    tempPlayer->setName(msg.readString(24));
    for (int i = 0; i < 6; i++) {
        tempPlayer->setAttributeBase(i + STR, msg.readInt8());
    }
    slot = msg.readInt8(); // character slot
    msg.readInt8();                        // unknown

    return tempPlayer;
}

void CharServerHandler::setCharSelectDialog(CharSelectDialog *window)
{
    mCharSelectDialog = window;
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

void CharServerHandler::getCharacters()
{
    connect();
}

void CharServerHandler::chooseCharacter(int slot, LocalPlayer *)
{
    MessageOut outMsg(CMSG_CHAR_SELECT);
    outMsg.writeInt8(slot);
}

void CharServerHandler::newCharacter(const std::string &name, int slot,
        bool gender, int hairstyle, int hairColor, std::vector<int> stats)
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

void CharServerHandler::deleteCharacter(int slot, LocalPlayer *character)
{
    MessageOut outMsg(CMSG_CHAR_DELETE);
    outMsg.writeInt32(character->getId());
    outMsg.writeString("a@a.com", 40);
}

void CharServerHandler::switchCharacter()
{
    // This is really a map-server packet
    MessageOut outMsg(CMSG_PLAYER_RESTART);
    outMsg.writeInt8(1);
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
