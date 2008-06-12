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

#include "shop.h"

#include "../utils/dtor.h"

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

void ShopItems::addItem(int inventoryIndex, short id, int amount, int price)
{
    ShopItem *item = new ShopItem(id, amount, price);
    item->setInvIndex(inventoryIndex);
    mShopItems.push_back(item);
}

void ShopItems::addItem(short id, int price)
{
    mShopItems.push_back(new ShopItem(id, 0, price));
}

ShopItem* ShopItems::at(int i) const
{
    return mShopItems.at(i);
}

void ShopItems::clear()
{
    std::for_each(mShopItems.begin(), mShopItems.end(), make_dtor(mShopItems));
    mShopItems.clear();
}

std::vector<ShopItem*>* ShopItems::getShop()
{
    return &mShopItems;
}
