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

#ifdef TMWSERV_SUPPORT
#define EQUIPMENT_SIZE 11
#else
#define EQUIPMENT_SIZE 10
#endif

class Item;

class Equipment
{
    public:
        /**
         * Constructor.
         */
        Equipment();

#ifdef TMWSERV_SUPPORT
        /**
         * Destructor.
         */
        ~Equipment();
#endif

        /**
         * Get equipment at the given slot.
         */
#ifdef TMWSERV_SUPPORT
        Item *getEquipment(int index)
#else
        int getEquipment(int index) const
#endif
        { return mEquipment[index]; }

#ifdef TMWSERV_SUPPORT
        /**
         * Clears equipment.
         */
        void clear();
#endif

        /**
         * Set equipment at the given slot.
         */
#ifdef TMWSERV_SUPPORT
        void setEquipment(int index, int id);
#else
        void setEquipment(int index, int inventoryIndex);
#endif

#ifdef EATHENA_SUPPORT
        /**
         * Remove equipment from the given slot.
         */
        void removeEquipment(int index);

        /**
         * Returns the item used in the arrow slot.
         */
        int getArrows() const { return mArrows; }

        /**
         * Set the item used in the arrow slot.
         */
        void setArrows(int arrows) { mArrows = arrows; }
#endif

    private:
#ifdef TMWSERV_SUPPORT
        Item *mEquipment[EQUIPMENT_SIZE];
#else
        int mEquipment[EQUIPMENT_SIZE];
        int mArrows;
#endif
};

#endif
