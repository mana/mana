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

#ifndef _SHOP_H
#define _SHOP_H

#include <string>
#include <vector>

#include <guichan/listmodel.hpp>

#include "../shopitem.h"

class ShopItem;

class ShopItems : public gcn::ListModel
{
    public:
        /**
         * Destructor.
         */
        ~ShopItems();

        /**
         * Adds an item to the list.
         */
        void addItem(int id, int amount, int price);

#ifdef EATHENA_SUPPORT
        /**
         * Adds an item to the list (used by eAthena sell dialog).
         */
        void addItem(int inventoryIndex, int id, int amount, int price);
#endif

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
