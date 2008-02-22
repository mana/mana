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

#include "messagein.h"
#include "network.h"
#include "protocol.h"

#include "../game.h"
#include "../localplayer.h"
#include "../log.h"
#include "../logindata.h"
#include "../main.h"

#include "../gui/ok_dialog.h"
#include "../gui/char_select.h"

CharServerHandler::CharServerHandler():
    mCharCreateDialog(0)
{
    static const Uint16 _messages[] = {
        0x006b,
        0x006c,
        0x006d,
        0x006e,
        0x006f,
        0x0070,
        0x0071,
        0x0081,
        0
    };
    handledMessages = _messages;
}

void CharServerHandler::handleMessage(MessageIn *msg)
{
    int slot;
    LocalPlayer *tempPlayer;

    logger->log("CharServerHandler: Packet ID: %x, Length: %d",
            msg->getId(), msg->getLength());
    switch (msg->getId())
    {
        case 0x006b:
            // Skip length word and an additional mysterious 20 bytes
            msg->skip(2 + 20);

            // Derive number of characters from message length
            n_character = (msg->getLength() - 24) / 106;

            for (int i = 0; i < n_character; i++)
            {
                tempPlayer = readPlayerData(*msg, slot);
                mCharInfo->select(slot);
                mCharInfo->setEntry(tempPlayer);
                logger->log("CharServer: Player: %s (%d)",
                        tempPlayer->getName().c_str(), slot);
            }

            state = CHAR_SELECT_STATE;
            break;

        case 0x006c:
            switch (msg->readInt8()) {
                case 0:
                    errorMessage = "Access denied";
                    break;
                case 1:
                    errorMessage = "Cannot use this ID";
                    break;
                default:
                    errorMessage = "Unknown failure to select character";
                    break;
            }
            mCharInfo->unlock();
            break;

        case 0x006d:
            tempPlayer = readPlayerData(*msg, slot);
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

        case 0x006e:
            new OkDialog("Error", "Failed to create character. Most likely"
                                  " the name is already taken.");

            if (mCharCreateDialog)
                mCharCreateDialog->unlock();
            break;

        case 0x006f:
            delete mCharInfo->getEntry();
            mCharInfo->setEntry(0);
            mCharInfo->unlock();
            n_character--;
            new OkDialog("Info", "Player deleted");
            break;

        case 0x0070:
            mCharInfo->unlock();
            new OkDialog("Error", "Failed to delete character.");
            break;

        case 0x0071:
            player_node = mCharInfo->getEntry();
            slot = mCharInfo->getPos();
            msg->skip(4); // CharID, must be the same as player_node->charID
            map_path = msg->readString(16);
            mLoginData->hostname = iptostring(msg->readInt32());
            mLoginData->port = msg->readInt16();
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
            state = CONNECTING_STATE;
            break;

        case 0x0081:
            switch (msg->readInt8()) {
                case 1:
                    errorMessage = "Map server offline";
                    break;
                case 3:
                    errorMessage = "Speed hack detected";
                    break;
                case 8:
                    errorMessage = "Duplicated login";
                    break;
                default:
                    errorMessage = "Unknown error with 0x0081";
                    break;
            }
            mCharInfo->unlock();
            state = ERROR_STATE;
            break;
    }
}

LocalPlayer *CharServerHandler::readPlayerData(MessageIn &msg, int &slot)
{
    LocalPlayer *tempPlayer = new LocalPlayer(mLoginData->account_ID, 0, NULL);
    tempPlayer->setGender(
            (mLoginData->sex == 0) ? GENDER_FEMALE : GENDER_MALE);

    tempPlayer->mCharId = msg.readInt32();
    tempPlayer->setXp(msg.readInt32());
    tempPlayer->mGp = msg.readInt32();
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
    tempPlayer->mHp = msg.readInt16();
    tempPlayer->mMaxHp = msg.readInt16();
    tempPlayer->mMp = msg.readInt16();
    tempPlayer->mMaxMp = msg.readInt16();
    msg.readInt16();                       // speed
    msg.readInt16();                       // class
    int hairStyle = msg.readInt16();
    Uint16 weapon = msg.readInt16();
    tempPlayer->setSprite(Being::WEAPON_SPRITE, weapon);
    tempPlayer->mLevel = msg.readInt16();
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
