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

#include "net/ea/equipmenthandler.h"

#include "net/ea/protocol.h"

#include "net/messagein.h"

#include "equipment.h"
#include "inventory.h"
#include "item.h"
#include "localplayer.h"
#include "log.h"

#include "gui/widgets/chattab.h"

#include "utils/gettext.h"

namespace EAthena {

enum { debugEquipment = 1 };

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

void EquipmentHandler::handleMessage(MessageIn &msg)
{
    int itemCount;
    int index, equipPoint, itemId;
    int type;
    int mask, position;
    Item *item;
    Inventory *inventory = player_node->getInventory();

    switch (msg.getId())
    {
        case SMSG_PLAYER_EQUIPMENT:
            msg.readInt16(); // length
            itemCount = (msg.getLength() - 4) / 20;

            for (int loop = 0; loop < itemCount; loop++)
            {
                index = msg.readInt16() - INVENTORY_OFFSET;
                itemId = msg.readInt16();
                msg.readInt8();  // type
                msg.readInt8();  // identify flag
                msg.readInt16(); // equip type
                equipPoint = msg.readInt16();
                msg.readInt8();  // attribute
                msg.readInt8();  // refine
                msg.skip(8);     // card

                if (debugEquipment)
                {
                    logger->log("Index: %d, ID: %d", index, itemId);
                }

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
                    player_node->mEquipment->setEquipment(position, index);
                }
            }
            break;

        case SMSG_PLAYER_EQUIP:
            index = msg.readInt16() - INVENTORY_OFFSET;
            equipPoint = msg.readInt16();
            type = msg.readInt8();

            if (!type)
            {
                localChatTab->chatLog(_("Unable to equip."), BY_SERVER);
                break;
            }

            // No point in searching when no point given
            if (!equipPoint)
                break;

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

            if (debugEquipment)
            {
                logger->log("Equipping: %i %i %i at position %i",
                            index, equipPoint, type, position);
            }

            item = inventory->getItem(player_node->mEquipment->getEquipment(position));

            // Unequip any existing equipped item in this position
            if (item)
                item->setEquipped(false);

            item = inventory->getItem(index);
            player_node->mEquipment->setEquipment(position, index);
            break;

        case SMSG_PLAYER_UNEQUIP:
            index = msg.readInt16() - INVENTORY_OFFSET;
            equipPoint = msg.readInt16();
            type = msg.readInt8();

            if (!type) {
                localChatTab->chatLog(_("Unable to unequip."), BY_SERVER);
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

            item = inventory->getItem(index);
            if (!item)
                break;

            item->setEquipped(false);

            if (equipPoint & 0x8000) {    // Arrows
                player_node->mEquipment->setArrows(-1);
            }
            else {
                player_node->mEquipment->removeEquipment(position);
            }

            if (debugEquipment)
            {
                logger->log("Unequipping: %i %i(%i) %i",
                            index, equipPoint, type, position);
            }
            break;

        case SMSG_PLAYER_ATTACK_RANGE:
            player_node->setAttackRange(msg.readInt16());
            break;

        case SMSG_PLAYER_ARROW_EQUIP:
            index = msg.readInt16();

            if (index <= 1)
                break;

            index -= INVENTORY_OFFSET;

            item = inventory->getItem(index);

            if (item) {
                item->setEquipped(true);
                player_node->mEquipment->setArrows(index);
                logger->log("Arrows equipped: %i", index);
            }
            break;
    }
}

} // namespace EAthena
