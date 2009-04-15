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

#ifndef _SHOPITEM_H
#define _SHOPITEM_H

#include <stack>

#include "item.h"

/**
 * Represents an item in a shop inventory. It can store quantity and inventory
 * indices of duplicate entries in the shop as well.
 */
class ShopItem : public Item
{
    public:
        /**
         * Constructor. Creates a new ShopItem.
         *
         * @param inventoryIndex the inventory index of the item
         * @param id the id of the item
         * @param quantity number of available copies of the item
         * @param price price of the item
         */
        ShopItem(int inventoryIndex, int id, int quantity, int price);

        /**
         * Constructor. Creates a new ShopItem. Inventory index will be set to
         * -1 and quantity to 0.
         *
         * @param id the id of the item
         * @param price price of the item
         */
        ShopItem(int id, int price);

        /**
         * Destructor.
         */
        ~ShopItem();

        /**
         * Add a duplicate. Id and price will be taken from this item.
         *
         * @param inventoryIndex the inventory index of the item
         * @param quantity number of available copies of the item
         */
        void addDuplicate(int inventoryIndex, int quantity);

        /**
         * Add a duplicate. Id and price will be taken from this item.
         * Needed for compatibility with ShopDuplicateItems (see) class
         * documentation).
         */
        void addDuplicate();

        /**
         * Gets the quantity of the currently topmost duplicate.
         *
         * @return the quantity of the currently topmost duplicate
         */
        int getCurrentQuantity() const
        {
            return mDuplicates.empty() ? 0 : mDuplicates.top()->quantity;
        }

        /**
         * Gets the inventory index of the currently topmost duplicate.
         *
         * @return the inventory index of the currently topmost duplicate
         */
        int getCurrentInvIndex() const
        {
            return mDuplicates.empty() ? mInvIndex :
                   mDuplicates.top()->inventoryIndex;
        }

        /**
         * Reduces the quantity of the topmost duplicate by the specified
         * amount. Also reduces the total quantity of this DuplicateItem.
         * Empty duplicates are automatically removed.
         *
         * If the amount is bigger than the quantity of the current topmost,
         * only sell as much as possible. Returns the amount actually sold (do
         * not ignore the return value!)
         *
         * @return the amount, that actually was sold.
         */
        int sellCurrentDuplicate(int quantity);

        /**
         * Gets the price of the item.
         *
         * @return the price of the item
         */
        int getPrice() const
        { return mPrice; }

        /**
         * Gets the display name for the item in the shop list.
         *
         * @return the display name for the item in the shop list
         */
        const std::string &getDisplayName() const
        { return mDisplayName; }

    protected:
        int mPrice;
        std::string mDisplayName;

        /**
         * Struct to keep track of duplicates.
         */
        typedef struct {
            int inventoryIndex;
            int quantity;
        } DuplicateItem;
        std::stack<DuplicateItem*> mDuplicates; /** <-- Stores duplicates */
};

#endif
