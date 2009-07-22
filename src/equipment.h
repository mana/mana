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

#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#define EQUIPMENT_SIZE 11

class Item;

class Equipment
{
    public:
        /**
         * Constructor.
         */
        Equipment();

        /**
         * Destructor.
         */
        ~Equipment();

        enum EquipmentSlots
        {
            EQUIP_TORSO_SLOT = 0,
            EQUIP_GLOVES_SLOT = 1,
            EQUIP_HEAD_SLOT = 2,
            EQUIP_LEGS_SLOT = 3,
            EQUIP_FEET_SLOT = 4,
            EQUIP_RING1_SLOT = 5,
            EQUIP_RING2_SLOT = 6,
            EQUIP_NECK_SLOT = 7,
            EQUIP_FIGHT1_SLOT = 8,
            EQUIP_FIGHT2_SLOT = 9,
            EQUIP_PROJECTILE_SLOT = 10,
            EQUIP_VECTOREND
        };

        /**
         * Get equipment at the given slot.
         */
        Item *getEquipment(int index)
        { return mEquipment[index]; }

        /**
         * Clears equipment.
         */
        void clear();

        /**
         * Set equipment at the given slot.
         */
        void setEquipment(int index, int id, int quantity = 0);

    private:
        Item *mEquipment[EQUIPMENT_SIZE];
};

#endif
