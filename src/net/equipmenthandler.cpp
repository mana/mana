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

#include "equipmenthandler.h"

#include "messagein.h"
#include "protocol.h"

#include "../being.h"
#include "../beingmanager.h"
#include "../equipment.h"
#include "../item.h"
#include "../localplayer.h"
#include "../log.h"

#include "../gui/chat.h"

EquipmentHandler::EquipmentHandler()
{
    static const Uint16 _messages[] = {
        SMSG_PLAYER_EQUIPMENT,
        SMSG_PLAYER_EQUIP,
        0x01d7,
        SMSG_PLAYER_UNEQUIP,
        SMSG_PLAYER_ARROW_EQUIP,
        0
    };
    handledMessages = _messages;
}

void EquipmentHandler::handleMessage(MessageIn *msg)
{
    Sint32 itemCount;
    Sint16 index, equipPoint, itemId;
    Sint8 type;
    int mask, position;
    Being *being;
    Item *item;

    switch (msg->getId())
    {
        case SMSG_PLAYER_EQUIPMENT:
            msg->readInt16(); // length
            itemCount = (msg->getLength() - 4) / 20;

            for (int loop = 0; loop < itemCount; loop++)
            {
                index = msg->readInt16();
                itemId = msg->readInt16();
                msg->readInt8();  // type
                msg->readInt8();  // identify flag
                msg->readInt16(); // equip type
                equipPoint = msg->readInt16();
                msg->readInt8();  // attribute
                msg->readInt8();  // refine
                msg->skip(8);     // card

                player_node->addInvItem(index, itemId, 1, true);

                if (equipPoint)
                {
                    mask = 1;
                    position = 0;
                    while (!(equipPoint & mask))
                    {
                        mask <<= 1;
                        position++;
                    }
                    item = player_node->getInvItem(index);
                    player_node->mEquipment->setEquipment(position, item);
                }
            }
            break;

        case SMSG_PLAYER_EQUIP:
            index = msg->readInt16();
            equipPoint = msg->readInt16();
            type = msg->readInt8();

            logger->log("Equipping: %i %i %i", index, equipPoint, type);

            if (!type) {
                chatWindow->chatLog("Unable to equip.", BY_SERVER);
                break;
            }

            if (!equipPoint) {
                // No point given, no point in searching
                break;
            }

            // Unequip any existing equipped item in this position
            mask = 1;
            position = 0;
            while (!(equipPoint & mask)) {
                mask <<= 1;
                position++;
            }
            logger->log("Position %i", position);
            item = player_node->mEquipment->getEquipment(position);
            if (item) {
                item->setEquipped(false);
            }

            item = player_node->getInvItem(index);
            player_node->mEquipment->setEquipment(position, item);
            player_node->setWeaponById(item->getId());
            break;

        case 0x01d7:
            // Equipment related
            being = beingManager->findBeing(msg->readInt32());
            msg->readInt8();  // equip point
            itemId = msg->readInt16();
            msg->readInt16(); // item id 2

            if (!being)
                break;

            being->setWeaponById(itemId);
            break;

        case SMSG_PLAYER_UNEQUIP:
            index = msg->readInt16();
            equipPoint = msg->readInt16();
            type = msg->readInt8();

            if (!type) {
                chatWindow->chatLog("Unable to unequip.", BY_SERVER);
                break;
            }

            if (!equipPoint) {
                // No point given, no point in searching
                break;
            }

            mask = 1;
            position = 0;
            while (!(equipPoint & mask)) {
                mask <<= 1;
                position++;
            }

            item = player_node->getInvItem(index);

            if (!item)
                break;

            item->setEquipped(false);

            switch (item->getId()) {
                case 529:
                case 1199:
                    player_node->mEquipment->setArrows(NULL);
                    break;
                case 521:
                case 522:
                case 530:
                case 536:
                case 1200:
                case 1201:
                    player_node->setWeapon(0);
                    // TODO: Why this break? Shouldn't a weapon be
                    //       unequipped in inventory too?
                    break;
                default:
                    player_node->mEquipment->removeEquipment(position);
                    break;
            }
            logger->log("Unequipping: %i %i(%i) %i",
                    index, equipPoint, type, position);
            break;

        case SMSG_PLAYER_ARROW_EQUIP:
            itemId = msg->readInt16();

            if (itemId <= 1)
                break;

            item = player_node->getInvItem(itemId);
            if (!item)
                break;

            item->setEquipped(true);
            player_node->mEquipment->setArrows(item);
            logger->log("Arrows equipped: %i", itemId);
            break;
    }
}
