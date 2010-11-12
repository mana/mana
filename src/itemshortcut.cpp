/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "configuration.h"
#include "event.h"
#include "inventory.h"
#include "item.h"
#include "itemshortcut.h"
#include "playerinfo.h"

#include "net/inventoryhandler.h"
#include "net/net.h"

#include "resources/iteminfo.h"

#include "utils/stringutils.h"

ItemShortcut *itemShortcut;

ItemShortcut::ItemShortcut():
    mItemSelected(-1)
{
    load();
}

ItemShortcut::~ItemShortcut()
{
    save();
}

void ItemShortcut::load()
{
    for (int i = 0; i < SHORTCUT_ITEMS; i++)
    {
        int itemId = (int) config.getValue("shortcut" + toString(i), -1);

        mItems[i] = itemId;
    }
}

void ItemShortcut::save()
{
    for (int i = 0; i < SHORTCUT_ITEMS; i++)
    {
        const int itemId = mItems[i] ? mItems[i] : -1;
        config.setValue("shortcut" + toString(i), itemId);
    }
}

void ItemShortcut::useItem(int index)
{
    if (mItems[index])
    {
        Item *item = PlayerInfo::getInventory()->findItem(mItems[index]);
        if (item && item->getQuantity())
        {
            if (item->getInfo().getEquippable())
            {
                if (item->isEquipped())
                    item->doEvent(EVENT_DOUNEQUIP);
                else
                    item->doEvent(EVENT_DOEQUIP);
            }
            else
            {
                item->doEvent(EVENT_DOUSE);
            }
        }
    }
}
