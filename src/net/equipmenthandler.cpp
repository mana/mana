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
 *  $Id: equipmenthandler.cpp 4347 2008-06-12 09:06:01Z b_lindeijer $
 */

#include "equipmenthandler.h"

#include "messagein.h"
#include "protocol.h"

#include "../beingmanager.h"
#include "../equipment.h"
#include "../inventory.h"
#include "../item.h"
#include "../localplayer.h"
#include "../log.h"

#include "../gui/chat.h"

EquipmentHandler::EquipmentHandler()
{
    static const Uint16 _messages[] = {
        SMSG_PLAYER_EQUIPMENT,
        SMSG_PLAYER_EQUIP,
        SMSG_PLAYER_UNEQUIP,
        SMSG_PLAYER_ARROW_EQUIP,
        SMSG_PLAYER_ATTACK_RANGE,
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
    Item *item;
    Inventory *inventory = player_node->getInventory();

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

                inventory->setItem(index, itemId, 1, true);

                if (equipPoint)
                {
                    mask = 1;
                    position = 0;
                    while (!(equipPoint & mask))
                    {
                        mask <<= 1;
                        position++;
                    }
                    item = inventory->getItem(index);
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

            /*
             * An item may occupy more than 1 slot.  If so, it's
             * only shown as equipped on the *first* slot.
             */
            mask = 1;
            position = 0;
            while (!(equipPoint & mask)) {
                mask <<= 1;
                position++;
            }
            logger->log("Position %i", position);
            item = player_node->mEquipment->getEquipment(position);

            // Unequip any existing equipped item in this position
            if (item) {
                item->setEquipped(false);
            }

            item = inventory->getItem(index);
            player_node->mEquipment->setEquipment(position, item);
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

            if (equipPoint & 0x8000) {    // Arrows
                player_node->mEquipment->setArrows(NULL);
                position = 11;
            } else {
                mask = 1;
                position = 0;
                while (!(equipPoint & mask)) {
                    mask <<= 1;
                    position++;
                }

                item = inventory->getItem(index);
                if (!item)
                    break;

                item->setEquipped(false);
                player_node->mEquipment->removeEquipment(position);
            }
            logger->log("Unequipping: %i %i(%i) %i",
                    index, equipPoint, type, position);
            break;

        case SMSG_PLAYER_ATTACK_RANGE:
            player_node->setAttackRange(msg->readInt16());
            break;

        case SMSG_PLAYER_ARROW_EQUIP:
            index = msg->readInt16();

            if (index <= 1)
                break;

            item = inventory->getItem(index);
            if (item) {
                item->setEquipped(true);
                player_node->mEquipment->setArrows(item);
                logger->log("Arrows equipped: %i", index);
            }
            break;
    }
}
