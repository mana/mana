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

#include "gui/widgets/shopitems.h"

#include "shopitem.h"

#include "utils/dtor.h"

ShopItems::ShopItems(bool mergeDuplicates) :
    mMergeDuplicates(mergeDuplicates)
{
}

ShopItems::~ShopItems()
{
    clear();
}

int ShopItems::getNumberOfElements()
{
    return mShopItems.size();
}

std::string ShopItems::getElementAt(int i)
{
    return mShopItems.at(i)->getDisplayName();
}

void ShopItems::addItem(int id, int amount, int price)
{
    mShopItems.push_back(new ShopItem(-1, id, amount, price));
}

void ShopItems::addItem(int inventoryIndex, int id, int quantity, int price)
{
    ShopItem *item = nullptr;
    if (mMergeDuplicates)
    {
        item = findItem(id);
    }

    if (item)
    {
        item->addDuplicate(inventoryIndex, quantity);
    }
    else
    {
        item = new ShopItem(inventoryIndex, id, quantity, price);
        mShopItems.push_back(item);
    }
}

ShopItem *ShopItems::at(int i) const
{
    return mShopItems.at(i);
}

void ShopItems::erase(int i)
{
    mShopItems.erase(mShopItems.begin() + i);
}

void ShopItems::clear()
{
    delete_all(mShopItems);
    mShopItems.clear();
}

ShopItem *ShopItems::findItem(int id)
{
    for (auto shopItem : mShopItems)
    {
        if (shopItem->getId() == id)
        {
            return shopItem;
        }
    }

    return nullptr;
}
