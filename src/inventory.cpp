/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#include <algorithm>

#include "inventory.h"
#include "item.h"
#include "log.h"

struct SlotUsed : public std::unary_function<Item*, bool>
{
    bool operator()(const Item *item) const
    {
        return item && item->getId() != -1 && item->getQuantity() > 0;
    }
};

Inventory::Inventory(int size):
    mSize(size)
{
    mItems = new Item*[mSize];
    std::fill_n(mItems, mSize, (Item*) 0);
}

Inventory::~Inventory()
{
    for (int i = 0; i < mSize; i++)
        delete mItems[i];

    delete [] mItems;
}

Item* Inventory::getItem(int index) const
{
    if (index < 0 || index >= INVENTORY_SIZE || !mItems[index] || mItems[index]->getQuantity() <= 0)
        return 0;

    return mItems[index];
}

Item* Inventory::findItem(int itemId) const
{
    for (int i = 0; i < mSize; i++)
        if (mItems[i] && mItems[i]->getId() == itemId)
            return mItems[i];

    return NULL;
}

void Inventory::addItem(int id, int quantity, bool equipment)
{
    setItem(getFreeSlot(), id, quantity, equipment);
}

void Inventory::setItem(int index, int id, int quantity, bool equipment)
{
    if (index < 0 || index >= mSize)
    {
        logger->log("Warning: invalid inventory index: %d", index);
        return;
    }

    if (!mItems[index] && id > 0)
    {
        Item *item = new Item(id, quantity, equipment);
        item->setInvIndex(index);
        mItems[index] = item;
    }
    else if (id > 0)
    {
        mItems[index]->setId(id);
        mItems[index]->setQuantity(quantity);
        mItems[index]->setEquipment(equipment);
    }
    else if (mItems[index])
    {
        removeItemAt(index);
    }
}

void Inventory::clear()
{
    for (int i = 0; i < mSize; i++)
        removeItemAt(i);
}

void Inventory::removeItem(int id)
{
    for (int i = 0; i < mSize; i++)
        if (mItems[i] && mItems[i]->getId() == id)
            removeItemAt(i);
}

void Inventory::removeItemAt(int index)
{
    delete mItems[index];
    mItems[index] = 0;
}

bool Inventory::contains(Item *item) const
{
    for (int i = 0; i < mSize; i++)
        if (mItems[i] && mItems[i]->getId() == item->getId())
            return true;

    return false;
}

int Inventory::getFreeSlot() const
{
    Item **i = std::find_if(mItems + 2, mItems + mSize,
            std::not1(SlotUsed()));
    return (i == mItems + mSize) ? -1 : (i - mItems);
}

int Inventory::getNumberOfSlotsUsed() const
{
    return count_if(mItems, mItems + mSize, SlotUsed());
}

int Inventory::getLastUsedSlot() const
{
    for (int i = mSize - 1; i >= 0; i--)
        if (SlotUsed()(mItems[i]))
            return i;

    return -1;
}

int Inventory::getInventorySize() const
{
    return INVENTORY_SIZE - 2;
}
