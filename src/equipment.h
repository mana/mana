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
 *  $Id: equipment.h 4347 2008-06-12 09:06:01Z b_lindeijer $
 */

#ifndef _TMW_EQUIPMENT_H_
#define _TMW_EQUIPMENT_H_

class Item;

#define EQUIPMENT_SIZE 10

class Equipment
{
    public:
        /**
         * Constructor.
         */
        Equipment();

        /**
         * Get equipment at the given slot.
         */
        Item* getEquipment(int index) const
        { return mEquipment[index]; }

        /**
         * Set equipment at the given slot.
         */
        void
        setEquipment(int index, Item *item);

        /**
         * Remove equipment from the given slot.
         */
        void
        removeEquipment(int index) { mEquipment[index] = 0; }

        /**
         * Remove the given item from equipment.
         */
        void removeEquipment(Item *item);

        /**
         * Get the item used in the arrow slot.
         */
        Item*
        getArrows() { return mArrows; }

        /**
         * Set the item used in the arrow slot.
         */
        void
        setArrows(Item *arrows) { mArrows = arrows; }

    private:
        Item *mEquipment[EQUIPMENT_SIZE];
        Item *mArrows;
};

#endif
