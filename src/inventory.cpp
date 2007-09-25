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

#include "inventory.h"

#include <algorithm>

#include "item.h"

struct SlotUsed : public std::unary_function<Item, bool>
{
    bool operator()(const Item &item) const {
        return (item.getId() && item.getQuantity());
    }
};

Inventory::Inventory()
{
    mItems = new Item[INVENTORY_SIZE];
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        mItems[i].setInvIndex(i);
    }
}

Inventory::~Inventory()
{
    delete [] mItems;
}

Item* Inventory::getItem(int index)
{
    if (index < 0 || index >= INVENTORY_SIZE)
    {
        return 0;
    }

    return &mItems[index];
}

void Inventory::addItem(int id, int quantity)
{
    addItem(getFreeSlot(), id, quantity);
}

void Inventory::addItem(int index, int id, int quantity)
{
    mItems[index].setId(id);
    mItems[index].increaseQuantity(quantity);
}


void Inventory::clear()
{
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        removeItemIndex(i);
    }
}

void Inventory::removeItem(int id)
{
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (mItems[i].getId() == id) {
            removeItemIndex(i);
        }
    }
}

void Inventory::removeItemIndex(int index)
{
    mItems[index].setId(0);
    mItems[index].setQuantity(0);
}

bool Inventory::contains(Item *item) const
{
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (mItems[i].getId() == item->getId()) {
            return true;
        }
    }

    return false;
}

int Inventory::getFreeSlot() const
{
    Item *i = std::find_if(mItems, mItems + INVENTORY_SIZE,
            std::not1(SlotUsed()));
    return (i == mItems + INVENTORY_SIZE) ? -1 : (i - mItems);
}

int Inventory::getNumberOfSlotsUsed() const
{
    return count_if(mItems, mItems + INVENTORY_SIZE, SlotUsed());
}

int Inventory::getLastUsedSlot() const
{
    for (int i = INVENTORY_SIZE - 1; i >= 0; i--) {
        if (SlotUsed()(mItems[i])) {
            return i;
        }
    }

    return -1;
}
