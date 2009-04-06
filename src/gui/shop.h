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

#ifndef SHOP_H
#define SHOP_H

#include <guichan/listmodel.hpp>

#include <string>
#include <vector>

class ShopItem;

/**
 * This class handles the list of items available in a shop.
 *
 * The addItem routine can automatically check, if an item already exists and
 * only adds duplicates to the old item, if one is found. The original
 * distribution of the duplicates can be retrieved from the item.
 *
 * This functionality can be enabled in the constructor.
 */
class ShopItems : public gcn::ListModel
{
    public:
        /**
         * Constructor.
         *
         * @param mergeDuplicates lets the Shop look for duplicate entries and
         *                        merges them to one item.
         */
        ShopItems(bool mergeDuplicates = false);

        ~ShopItems();

        /**
         * Adds an item to the list.
         */
        void addItem(int id, int amount, int price);

        /**
         * Adds an item to the list (used by sell dialog). Looks for
         * duplicate entries, if mergeDuplicates was turned on.
         *
         * @param inventoryIndex the inventory index of the item
         * @param id the id of the item
         * @param quantity number of available copies of the item
         * @param price price of the item
         */
        void addItem(int inventoryIndex, int id, int amount, int price);

        /**
         * Returns the number of items in the shop.
         */
        int getNumberOfElements();

        /**
         * Returns the name of item number i in the shop.
         *
         * @param i the index to retrieve
         */
        std::string getElementAt(int i);

        /**
         * Returns the item number i in the shop.
         */
        ShopItem *at(int i) const;

        /**
         * Removes an element from the shop.
         *
         * @param i index to remove
         */
        void erase(int i);

        /**
         * Clears the list of items in the shop.
         */
        void clear();

    private:
        /**
         * Searches the current items in the shop for the specified
         * id and returns the item if found, or 0 else.
         * 
         * @return the item found or 0
         */
        ShopItem *findItem(int id);

        /** The list of items in the shop. */
        std::vector<ShopItem*> mShopItems;

        /** Look for duplicate entries on addition. */
        bool mMergeDuplicates;
};

#endif // SHOP_H
