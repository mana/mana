/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "inventory.h"
#include "item.h"
#include "log.h"

#include "net/inventoryhandler.h"
#include "net/net.h"

#include <algorithm>
#include <functional>

static bool slotUsed(const Item *item)
{
    return item && item->getId() >= 0 && item->getQuantity() > 0;
}

Inventory::Inventory(Type type, int size):
    mType(type),
    mSize(size == -1 ? Net::getInventoryHandler()->getSize(type) : size),
    mUsed(0)
{
    mItems = new Item*[mSize];
    std::fill_n(mItems, mSize, (Item*) nullptr);
}

Inventory::~Inventory()
{
    for (int i = 0; i < mSize; i++)
        delete mItems[i];

    delete [] mItems;
}

Item *Inventory::getItem(int index) const
{
    if (index < 0 || index >= mSize || !mItems[index] || mItems[index]->getQuantity() <= 0)
        return nullptr;

    return mItems[index];
}

Item *Inventory::findItem(int itemId) const
{
    for (int i = 0; i < mSize; i++)
        if (mItems[i] && mItems[i]->getId() == itemId)
            return mItems[i];

    return nullptr;
}

void Inventory::addItem(int id, int quantity)
{
    setItem(getFreeSlot(), id, quantity);
}

void Inventory::setItem(int index, int id, int quantity)
{
    if (index < 0 || index >= mSize)
    {
        logger->log("Warning: invalid inventory index: %d", index);
        return;
    }

    if (!mItems[index] && id > 0)
    {
        Item *item = new Item(id, quantity);
        item->setInvIndex(index);
        mItems[index] = item;
        mUsed++;
        distributeSlotsChangedEvent();
    }
    else if (id > 0)
    {
        mItems[index]->setId(id);
        mItems[index]->setQuantity(quantity);
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
    mItems[index] = nullptr;
    if (mUsed > 0) {
        mUsed--;
        distributeSlotsChangedEvent();
    }
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
    Item **i = std::find_if(mItems, mItems + mSize, std::not_fn(slotUsed));
    return (i == mItems + mSize) ? -1 : (i - mItems);
}

int Inventory::getLastUsedSlot() const
{
    for (int i = mSize - 1; i >= 0; i--)
        if (slotUsed(mItems[i]))
            return i;

    return -1;
}

void Inventory::addInventoryListener(InventoryListener* listener)
{
    mInventoryListeners.push_back(listener);
}

void Inventory::removeInventoryListener(InventoryListener* listener)
{
    mInventoryListeners.remove(listener);
}

void Inventory::distributeSlotsChangedEvent()
{
    InventoryListenerList::const_iterator i = mInventoryListeners.begin();
    InventoryListenerList::const_iterator i_end = mInventoryListeners.end();
    for (; i != i_end; i++)
    {
        (*i)->slotsChanged(this);
    }
}
