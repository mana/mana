/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef INVENTORYHANDLER_H
#define INVENTORYHANDLER_H

#include "item.h"

#include <iosfwd>

namespace Net {

class InventoryHandler
{
    public:
        virtual void equipItem(const Item *item) = 0;

        virtual void unequipItem(const Item *item) = 0;

        virtual void useItem(const Item *item) = 0;

        virtual void dropItem(const Item *item, int amount) = 0;

        virtual bool canSplit(const Item *item) = 0;

        virtual void splitItem(const Item *item, int amount) = 0;

        virtual void moveItem(int oldIndex, int newIndex) = 0;

        virtual void openStorage() = 0;

        virtual void closeStorage() = 0;

        //void changeCart() = 0;

        enum StorageType {
            INVENTORY,
            STORAGE,
            CART
        };

        virtual void moveItem(StorageType source, int slot, int amount,
                              StorageType destination) = 0;
};

} // namespace Net

#endif // INVENTORYHANDLER_H
