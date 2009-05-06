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

#ifndef INVENTORY_H
#define INVENTORY_H

class Item;

#ifdef EATHENA_SUPPORT
const int INVENTORY_SIZE = 100;
#else
const int INVENTORY_SIZE = 50;
#endif
const int STORAGE_SIZE = 301;

class Inventory
{
    public:
        /**
         * Constructor.
         *
         * @param size the number of items that fit in the inventory
         */
        Inventory(int size);

        /**
         * Destructor.
         */
        ~Inventory();

        /**
         * Returns the size that this instance is configured for.
         */
        int getSize() const { return mSize; }

        /**
         * Returns the item at the specified index.
         */
        Item *getItem(int index) const;

        /**
         * Searches for the specified item by it's id.
         *
         * @param itemId The id of the item to be searched.
         * @return Item found on success, NULL on failure.
         */
        Item *findItem(int itemId) const;

        /**
         * Adds a new item in a free slot.
         */
        void addItem(int id, int quantity, bool equipment = false);

        /**
         * Sets the item at the given position.
         */
        void setItem(int index, int id, int quantity, bool equipment = false);

        /**
         * Remove a item from the inventory.
         */
        void removeItem(int id);

        /**
         * Remove the item at the specified index from the inventory.
         */
        void removeItemAt(int index);

        /**
         * Checks if the given item is in the inventory.
         */
        bool contains(Item *item) const;

        /**
         * Returns id of next free slot or -1 if all occupied.
         */
        int getFreeSlot() const;

        /**
         * Reset all item slots.
         */
        void clear();

        /**
         * Get the number of slots filled with an item
         */
        int getNumberOfSlotsUsed() const;

        /**
         * Returns the index of the last occupied slot or 0 if none occupied.
         */
        int getLastUsedSlot() const;

        static const int NO_SLOT_INDEX = -1; /**< Slot has no index. */

    protected:
        Item **mItems;  /**< The holder of items */
        int mSize;      /**< The max number of inventory items */
};

#endif
