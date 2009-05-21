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

#include "net/ea/network.h"
#include "net/ea/protocol.h"

#include "net/logindata.h"
#include "net/messagein.h"
#include "net/messageout.h"

#include "game.h"
#include "log.h"
#include "main.h"

#include "gui/charcreatedialog.h"
#include "gui/okdialog.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

Net::CharHandler *charHandler;

namespace EAthena {

CharServerHandler::CharServerHandler():
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

void CharServerHandler::handleMessage(MessageIn &msg)
{
    int slot, flags;
    LocalPlayer *tempPlayer;

    logger->log("CharServerHandler: Packet ID: %x, Length: %d",
            msg.getId(), msg.getLength());
    switch (msg.getId())
    {
        case 0x006b:
            msg.skip(2); // Length word
            flags = msg.readInt32(); // Aethyra extensions flags
            logger->log("Server flags are: %x", flags);
            msg.skip(16); // Unused

            // Derive number of characters from message length
            n_character = (msg.getLength() - 24) / 106;

            for (int i = 0; i < n_character; i++)
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
                    errorMessage = _("Access denied");
                    break;
                case 1:
                    errorMessage = _("Cannot use this ID");
                    break;
                default:
                    errorMessage = _("Unknown failure to select character");
                    break;
            }
            mCharInfo->unlock();
            break;

        case SMSG_CHAR_CREATE_SUCCEEDED:
            tempPlayer = readPlayerData(msg, slot);
            mCharInfo->unlock();
            mCharInfo->select(slot);
            mCharInfo->setEntry(tempPlayer);
            n_character++;

            // Close the character create dialog
            if (mCharCreateDialog)
            {
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
            delete mCharInfo->getEntry();
            mCharInfo->setEntry(0);
            mCharInfo->unlock();
            n_character--;
            new OkDialog(_("Info"), _("Character deleted."));
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
            mLoginData->hostname = ipToString(msg.readInt32());
            mLoginData->port = msg.readInt16();
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
            state = STATE_CONNECTING;
            break;
    }
}

LocalPlayer *CharServerHandler::readPlayerData(MessageIn &msg, int &slot)
{
    LocalPlayer *tempPlayer = new LocalPlayer(mLoginData->account_ID, 0, NULL);
    tempPlayer->setGender(mLoginData->sex);

    tempPlayer->mCharId = msg.readInt32();
    tempPlayer->setXp(msg.readInt32());
    tempPlayer->setMoney(msg.readInt32());
    tempPlayer->mJobXp = msg.readInt32();
    tempPlayer->mJobLevel = msg.readInt32();
    tempPlayer->setSprite(Being::SHOE_SPRITE, msg.readInt16());
    tempPlayer->setSprite(Being::GLOVES_SPRITE, msg.readInt16());
    tempPlayer->setSprite(Being::CAPE_SPRITE, msg.readInt16());
    tempPlayer->setSprite(Being::MISC1_SPRITE, msg.readInt16());
    msg.readInt32();                       // option
    msg.readInt32();                       // karma
    msg.readInt32();                       // manner
    msg.skip(2);                          // unknown
    tempPlayer->setHp(msg.readInt16());
    tempPlayer->setMaxHp(msg.readInt16());
    tempPlayer->mMp = msg.readInt16();
    tempPlayer->mMaxMp = msg.readInt16();
    msg.readInt16();                       // speed
    msg.readInt16();                       // class
    int hairStyle = msg.readInt16();
    Uint16 weapon = msg.readInt16();
    tempPlayer->setSprite(Being::WEAPON_SPRITE, weapon);
    tempPlayer->setLevel(msg.readInt16());
    msg.readInt16();                       // skill point
    tempPlayer->setSprite(Being::BOTTOMCLOTHES_SPRITE, msg.readInt16()); // head bottom
    tempPlayer->setSprite(Being::SHIELD_SPRITE, msg.readInt16());
    tempPlayer->setSprite(Being::HAT_SPRITE, msg.readInt16()); // head option top
    tempPlayer->setSprite(Being::TOPCLOTHES_SPRITE, msg.readInt16()); // head option mid
    int hairColor = msg.readInt16();
    tempPlayer->setHairStyle(hairStyle, hairColor);
    tempPlayer->setSprite(Being::MISC2_SPRITE, msg.readInt16());
    tempPlayer->setName(msg.readString(24));
    for (int i = 0; i < 6; i++) {
        tempPlayer->mAttr[i] = msg.readInt8();
    }
    slot = msg.readInt8(); // character slot
    msg.readInt8();                        // unknown

    return tempPlayer;
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

    mCharCreateDialog->setAttributes(attributes, 30, 1, 9);
    mCharCreateDialog->setFixedGender(true, mLoginData->sex);
}

void CharServerHandler::connect(LoginData *loginData)
{
    mLoginData = loginData;
    
    MessageOut outMsg(CMSG_CHAR_SERVER_CONNECT);
    outMsg.writeInt32(loginData->account_ID);
    outMsg.writeInt32(loginData->session_ID1);
    outMsg.writeInt32(loginData->session_ID2);
    // [Fate] The next word is unused by the old char server, so we squeeze in
    //        tmw client version information
    outMsg.writeInt16(CLIENT_PROTOCOL_VERSION);
    outMsg.writeInt8((loginData->sex == GENDER_MALE) ? 1 : 0);

    // We get 4 useless bytes before the real answer comes in (what are these?)
    mNetwork->skip(4);
}

void CharServerHandler::chooseCharacter(int slot, LocalPlayer* character)
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

void CharServerHandler::deleteCharacter(int slot, LocalPlayer* character)
{
    MessageOut outMsg(CMSG_CHAR_DELETE);
    outMsg.writeInt32(character->mCharId);
    outMsg.writeString("a@a.com", 40);
}

} // namespace EAthena
