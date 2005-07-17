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

#ifndef _ITEM_H
#define _ITEM_H

#include "resources/iteminfo.h"

class Item
{
    public:
        Item(int id=-1, int quantity=0,
                bool equipment=false, bool equipped=false);

        ~Item();

        void setId(int id);
        int getId();

        void setQuantity(int quantity);
        void increaseQuantity(int amount);
        int getQuantity();

        void setEquipment(bool equipment);
        bool isEquipment();

        void setEquipped(bool equipped);
        bool isEquipped();

        int getInvIndex();
        void setInvIndex(int index);

        ItemInfo* getInfo();

    protected:
        int id;
        int quantity;
        bool equipment;
        bool equipped;

        int invIndex;
};

inline void Item::setId(int id)
{
    this->id = id;
}

inline int Item::getId()
{
    return id;
}

inline void Item::setQuantity(int quantity)
{
    this->quantity = quantity;
}

inline void Item::increaseQuantity(int amount)
{
    this->quantity += amount;
}

inline int Item::getQuantity()
{
    return quantity;
}

inline void Item::setEquipment(bool equipment)
{
    this->equipment = equipment;
}

inline bool Item::isEquipment()
{
    return equipment;
}

inline void Item::setEquipped(bool equipped)
{
    this->equipped = equipped;
}

inline bool Item::isEquipped()
{
    return equipped;
}

inline int Item::getInvIndex()
{
    return invIndex;
}

inline void Item::setInvIndex(int index)
{
    this->invIndex = index;
}

#endif
