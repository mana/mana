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

#include "shopitem.h"

#include "units.h"

#include "resources/iteminfo.h"

ShopItem::ShopItem(int inventoryIndex, int id,
                   int quantity, int price) :
    Item(id, 0),
    mPrice(price)
{
    mDisplayName = getInfo().name +
                        " (" + Units::formatCurrency(mPrice) + ")";
    setInvIndex(inventoryIndex);
    addDuplicate(inventoryIndex, quantity);
}

ShopItem::~ShopItem() = default;

void ShopItem::addDuplicate(int inventoryIndex, int quantity)
{
    DuplicateItem &di = mDuplicates.emplace();
    di.inventoryIndex = inventoryIndex;
    di.quantity = quantity;
    mQuantity += quantity;
}

int ShopItem::sellCurrentDuplicate(int quantity)
{
    DuplicateItem &dupl = mDuplicates.top();
    int sellCount = quantity <= dupl.quantity ? quantity : dupl.quantity;
    dupl.quantity -= sellCount;
    mQuantity -= sellCount;
    if (dupl.quantity == 0)
        mDuplicates.pop();
    return sellCount;
}
