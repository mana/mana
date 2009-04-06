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

#include "gui/shop.h"

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
    ShopItem *item = 0;
    if (mMergeDuplicates)
    {
        item = findItem(id);
    }

    if (item)
    {
        item->addDuplicate (inventoryIndex, quantity);
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
    ShopItem *item;

    std::vector<ShopItem*>::iterator it;
    for(it = mShopItems.begin(); it != mShopItems.end(); it++)
    {
        item = *(it);
        if (item->getId() == id)
        {
            return item;
        }
    }

    return 0;
}
