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

static bool slotUsed(const Item *item)
{
    return item && item->getId() >= 0 && item->getQuantity() > 0;
}

Inventory::Inventory(Type type, int size)
    : mType(type)
{
    mItems.resize(size == -1 ? Net::getInventoryHandler()->getSize(type) : size);
}

Inventory::~Inventory() = default;

Item *Inventory::getItem(int index) const
{
    if (index < 0 || index >= getSize() || !mItems[index] || mItems[index]->getQuantity() <= 0)
        return nullptr;

    return mItems[index].get();
}

Item *Inventory::findItem(int itemId) const
{
    for (auto &item : mItems)
        if (item && item->getId() == itemId)
            return item.get();

    return nullptr;
}

void Inventory::addItem(int id, int quantity)
{
    setItem(getFreeSlot(), id, quantity);
}

void Inventory::setItem(int index, int id, int quantity)
{
    if (index < 0 || index >= getSize())
    {
        Log::warn("Invalid inventory index: %d", index);
        return;
    }

    if (id > 0)
    {
        if (!mItems[index])
        {
            auto item = std::make_unique<Item>(id, quantity);
            item->setInvIndex(index);
            mItems[index] = std::move(item);
            mUsed++;
            distributeSlotsChangedEvent();
        }
        else
        {
            mItems[index]->setId(id);
            mItems[index]->setQuantity(quantity);
        }
    }
    else if (mItems[index])
    {
        removeItemAt(index);
    }
}

void Inventory::clear()
{
    for (auto &item : mItems)
        item = nullptr;
    mUsed = 0;
    distributeSlotsChangedEvent();
}

void Inventory::removeItemAt(int index)
{
    mItems[index] = nullptr;
    if (mUsed > 0) {
        mUsed--;
        distributeSlotsChangedEvent();
    }
}

bool Inventory::contains(Item *item) const
{
    return std::any_of(mItems.begin(),
                       mItems.end(),
                       [id = item->getId()](auto &i) {
        return i && i->getId() == id;
    });
}

int Inventory::getFreeSlot() const
{
    for (int i = 0; i < getSize(); i++)
        if (!slotUsed(mItems[i].get()))
            return i;

    return -1;
}

int Inventory::getLastUsedSlot() const
{
    for (int i = getSize() - 1; i >= 0; i--)
        if (slotUsed(mItems[i].get()))
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
    for (auto inventoryListener : mInventoryListeners)
        inventoryListener->slotsChanged(this);
}
