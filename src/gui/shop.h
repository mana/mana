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

#ifndef _SHOP_H
#define _SHOP_H

#include <string>
#include <vector>

#include <guichan/listmodel.hpp>
#include "../resources/image.h"

struct ITEM_SHOP {
    short id;
    std::string name;
    Image *image;
    int price;
    int index;
    int quantity;
};

class ShopItems : public gcn::ListModel
{
    public:
        /**
         * Destructor
         */
        ~ShopItems();

        /**
         * Adds an item and its associated picture
         */
        void addItem(short id, int price);

        /**
         * Convenience function for adding items
         */
        void push_back(ITEM_SHOP item_shop);

        /**
         * Returns the number of items in the shop.
         */
        int getNumberOfElements();

        /**
         * Returns the name of item number i in the shop.
         */
        std::string getElementAt(int i);

        /**
         * Returns the item number i in the shop.
         */
        ITEM_SHOP at(int i);

        /**
         * Clear the vector.
         */
        void clear();

        /**
         * Direct access to the vector
         */
        std::vector<ITEM_SHOP>* getShop();

    private:
        std::vector<ITEM_SHOP> mItemsShop;

};

#endif
