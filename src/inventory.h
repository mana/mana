/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#pragma once

#include <list>
#include <memory>
#include <vector>

class Inventory;
class Item;

class InventoryListener
{
public:
    virtual ~InventoryListener() = default;

    virtual void slotsChanged(Inventory* inventory) = 0;

protected:
    InventoryListener() {}
};

class Inventory
{
    public:
        static const int NO_SLOT_INDEX = -1; /**< Slot has no index. */

        enum Type {
            INVENTORY,
            STORAGE,
            TRADE,
            TYPE_END
        };


        /**
         * Constructor.
         *
         * @param size the number of items that fit in the inventory
         */
        Inventory(Type type, int size = -1);

        ~Inventory();

        /**
         * Returns the size that this instance is configured for.
         */
        int getSize() const { return mItems.size(); }

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
        void addItem(int id, int quantity);

        /**
         * Sets the item at the given position.
         */
        void setItem(int index, int id, int quantity);

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
        int getNumberOfSlotsUsed() const
        { return mUsed; }

        /**
         * Returns the index of the last occupied slot or 0 if none occupied.
         */
        int getLastUsedSlot() const;

        void addInventoryListener(InventoryListener* listener);
        void removeInventoryListener(InventoryListener* listener);

        int getType() const
        { return mType; }

        bool isMainInventory() const
        { return mType == INVENTORY; }

    protected:
        std::list<InventoryListener *> mInventoryListeners;

        void distributeSlotsChangedEvent();

        Type mType;
        std::vector<std::unique_ptr<Item>> mItems;  /**< The holder of items */
        int mUsed = 0;  /**< The number of slots in use */
};
