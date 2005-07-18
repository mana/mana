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

#ifndef _INVENTORY_H
#define _INVENTORY_H

#include "item.h"

#define INVENTORY_SIZE 100

class Inventory
{
    public:
        Inventory();
        ~Inventory();

        /**
         * Returns the item at the specified index.
         */
        Item* getItem(int index);

        /**
         * Adds a new item in a free slot.
         */
        void addItem(int id, int quantity, bool equipment);

        /**
         * Adds a new item at a given position.
         */
        void addItem(int index, int id, int quantity, bool equipment);

        /**
         * Remove a item from the inventory.
         */
        void removeItem(int id);

        /**
         * Checks if the given item is in the inventory
         */
        bool contains(Item *item);

        /**
         * Equips an item.
         */
        void equipItem(Item *item);

        /**
         * Unequips an item.
         */
        void unequipItem(Item *item);

        int useItem(Item *item);

        int dropItem(Item *item, int quantity);

        /**
         * Returns id of next free slot or -1 if all occupied.
         */
        int getFreeSlot();

        /**
         * Reset all item slots.
         */
        void resetItems();

        /**
         * Get the number of slots filled with an item
         */
        int getNumberOfSlotsUsed();

    protected:
        Item items[INVENTORY_SIZE];  /**< The holder of items */
};

inline Item* Inventory::getItem(int index)
{
    return &items[index];
}

inline void Inventory::addItem(int id, int quantity, bool equipment)
{
    addItem(getFreeSlot(), id, quantity, equipment);
}

inline void Inventory::addItem(int index, int id, int quantity, bool equipment)
{
    items[index].setId(id);
    items[index].increaseQuantity(quantity);
    items[index].setEquipment(equipment);
}

extern Inventory *inventory;

#endif
