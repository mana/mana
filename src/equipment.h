/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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
        int getEquipment(int index) const
        { return mEquipment[index]; }

        /**
         * Set equipment at the given slot.
         */
        void setEquipment(int index, int inventoryIndex);

        /**
         * Remove equipment from the given slot.
         */
        void removeEquipment(int index) { if (index >= 0 && index < EQUIPMENT_SIZE) mEquipment[index] = 0; }

        /**
         * Returns the item used in the arrow slot.
         */
        int getArrows() const { return mArrows; }

        /**
         * Set the item used in the arrow slot.
         */
        void setArrows(int arrows) { mArrows = arrows; }

    private:
        int mEquipment[EQUIPMENT_SIZE];
        int mArrows;
};

#endif
