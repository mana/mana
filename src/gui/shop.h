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
 */

#ifndef _SHOP_H
#define _SHOP_H

#include <string>
#include <vector>

#include <guichan/listmodel.hpp>

#include "../guichanfwd.h"
#include "../shopitem.h"

#include "../resources/image.h"

class ShopItems : public gcn::ListModel
{
    public:
        /**
         * Destructor.
         */
        ~ShopItems();

        /**
         * Adds an item to the list (used by sell dialog).
         */
        void addItem(int inventoryIndex, short id, int amount, int price);

        /**
         * Adds an item to the list (used by buy dialog).
         */
        void addItem(short id, int price);

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
        ShopItem* at(int i) const;

        /**
         * Clear the vector.
         */
        void clear();

        /**
         * Direct access to the vector.
         */
        std::vector<ShopItem*>* getShop();

    private:
        std::vector<ShopItem*> mShopItems;
};

#endif
