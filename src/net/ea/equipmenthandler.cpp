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

const Equipment::EquipmentSlots EQUIP_POINTS[Equipment::EQUIP_VECTOREND] = {
    Equipment::EQUIP_LEGS_SLOT,
    Equipment::EQUIP_FIGHT1_SLOT,
    Equipment::EQUIP_GLOVES_SLOT,
    Equipment::EQUIP_RING2_SLOT,
    Equipment::EQUIP_RING1_SLOT,
    Equipment::EQUIP_FIGHT2_SLOT,
    Equipment::EQUIP_FEET_SLOT,
    Equipment::EQUIP_NECK_SLOT,
    Equipment::EQUIP_HEAD_SLOT,
    Equipment::EQUIP_TORSO_SLOT,
    Equipment::EQUIP_PROJECTILE_SLOT};

Item *equips[Equipment::EQUIP_VECTOREND];

namespace EAthena {

enum { debugEquipment = 1 };

void setEquipment(int eAthenaSlot, int index, bool equiped)
{
    if (!eAthenaSlot)
        return;

    Item *item = player_node->getInventory()->getItem(index);

    if (!item)
        return;

    int position = 0;

    if (eAthenaSlot & 0x8000) {    // Arrows
        position = Equipment::EQUIP_PROJECTILE_SLOT;
    }
    else
    {
        /*
         * An item may occupy more than 1 slot.  If so, it's
         * only shown as equipped on the *first* slot.
         */
        int mask = 1;
        while (!(eAthenaSlot & mask))
        {
            mask <<= 1;
            position++;
        }

        position = EQUIP_POINTS[position];
    }

    if (equips[position])
        equips[position]->setEquipped(false);

    if (equiped && item)
    {
        equips[position] = item;
        item->setEquipped(true);
        player_node->mEquipment->setEquipment(position, item->getId(), item->getQuantity());

        if (debugEquipment)
        {
            logger->log("Equipping: %i %i at position %i",
                        index, eAthenaSlot, position);
        }
    }
    else
    {
        equips[position] = NULL;
        player_node->mEquipment->setEquipment(position, -1);

        if (debugEquipment)
        {
            logger->log("Unequipping: %i %i at position %i",
                        index, eAthenaSlot, position);
        }
    }
}

Item *getRealEquipedItem(const Item *equipped)
{
    if (!equipped)
        return NULL;

    for (int i = 0; i < Equipment::EQUIP_VECTOREND; i++)
    {
        if (equips[i] && equipped->getId() == equips[i]->getId())
            return equips[i];
    }

    return NULL;
}

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
    memset(equips, 0, sizeof(equips));
}

void EquipmentHandler::handleMessage(MessageIn &msg)
{
    int itemCount;
    int index, equipPoint, itemId;
    int type;
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

                inventory->setItem(index, itemId, 1, true);

                setEquipment(equipPoint, index, true);
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

            setEquipment(equipPoint, index, true);
            break;

        case SMSG_PLAYER_UNEQUIP:
            index = msg.readInt16() - INVENTORY_OFFSET;
            equipPoint = msg.readInt16();
            type = msg.readInt8();

            if (!type) {
                localChatTab->chatLog(_("Unable to unequip."), BY_SERVER);
                break;
            }

            setEquipment(equipPoint, index, false);
            break;

        case SMSG_PLAYER_ATTACK_RANGE:
            player_node->setAttackRange(msg.readInt16());
            break;

        case SMSG_PLAYER_ARROW_EQUIP:
            index = msg.readInt16();

            if (index <= 1)
                break;

            index -= INVENTORY_OFFSET;

            logger->log("Arrows equipped: %i", index);
            setEquipment(0x8000, index, true);
            break;
    }
}

} // namespace EAthena
