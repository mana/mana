/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef INVENTORYHANDLER_H
#define INVENTORYHANDLER_H

#include "inventory.h"
#include "item.h"
#include "position.h"

#include <iosfwd>

namespace Net {

// Default positions of the boxes, 2nd dimension is X and Y respectively.
const int fallBackBoxesPosition[][2] = {
    { 90,  40 },    // EQUIP_TORSO_SLOT
    { 8,   78 },    // EQUIP_GLOVES_SLOT
    { 70,  0 },     // EQUIP_HEAD_SLOT
    { 50,  208 },   // EQUIP_LEGS_SLOT
    { 90,  208 },   // EQUIP_FEET_SLOT
    { 8,   168 },   // EQUIP_RING1_SLOT
    { 129, 168 },   // EQUIP_RING2_SLOT
    { 50,  40 },    // EQUIP_NECK_SLOT
    { 8,   123 },   // EQUIP_FIGHT1_SLOT
    { 129, 123 },   // EQUIP_FIGHT2_SLOT
    { 129, 78 }     // EQUIP_PROJECTILE_SLOT
};

const std::string fallBackBoxesBackground[] = {
    "equip-box-chest.png",
    "equip-box-hands.png",
    "equip-box-head.png",
    "equip-box-legs.png",
    "equip-box-feet.png",
    "equip-box-ring.png",
    "equip-box-ring.png",
    "equip-box-neck.png",
    "equip-box-weapon.png",
    "equip-box-shield.png",
    "equip-box-ammo.png"
};

static const std::string empty = std::string();

class InventoryHandler
{
    public:
        virtual ~InventoryHandler() {}

        virtual bool canSplit(const Item *item) = 0;

        // TODO: fix/remove me
        virtual size_t getSize(int type) const = 0;

        virtual std::string getSlotName(int slotIndex) const
        { return std::string(); }

        virtual bool isWeaponSlot(unsigned int slotTypeId) const = 0;

        virtual bool isAmmoSlot(unsigned int slotTypeId) const = 0;

        virtual unsigned int getVisibleSlotsNumber() const
        { return 0; }

        virtual Position getBoxPosition(unsigned int slotIndex) const
        {
            if (slotIndex < (sizeof(fallBackBoxesPosition)
                             / sizeof(fallBackBoxesPosition[0][0])))
                return Position(fallBackBoxesPosition[slotIndex][0],
                                fallBackBoxesPosition[slotIndex][1]);
            return Position(0,0);
        }

        virtual const std::string& getBoxBackground(unsigned int slotIndex) const
        {
            if (slotIndex < sizeof(fallBackBoxesBackground))
                return fallBackBoxesBackground[slotIndex];
            return empty; // The empty string
        }
};

} // namespace Net

#endif // INVENTORYHANDLER_H
