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
class InvyHandler
{
    public:
        virtual void equipItem(Item *item) {}

        virtual void unequipItem(Item *item) {}

        virtual void useItem(int slot, Item *item) {}

        virtual void dropItem(int slot, int amount) {}

        virtual void splitItem(int slot, int amount) {}

        virtual void openStorage() {}

        virtual void closeStorage() {}

        //void changeCart() {}

        enum StorageType {
            INVENTORY,
            STORAGE,
            CART
        };

        virtual void moveItem(StorageType source, int slot, int amount,
                 StorageType destination) {}
};
}

#endif // INVENTORYHANDLER_H
