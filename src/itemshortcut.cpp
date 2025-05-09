/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
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

#include "itemshortcut.h"

#include "configuration.h"
#include "event.h"
#include "inventory.h"
#include "item.h"
#include "playerinfo.h"

ItemShortcut *itemShortcut;

ItemShortcut::ItemShortcut()
{
    load();
}

ItemShortcut::~ItemShortcut()
{
    save();
}

void ItemShortcut::load()
{
    for (int &item : mItems)
        item = -1;

    for (auto &shortcut : config.itemShortcuts)
    {
        if (shortcut.index >= 0 && shortcut.index < SHORTCUT_ITEMS)
            mItems[shortcut.index] = shortcut.itemId;
    }
}

void ItemShortcut::save()
{
    config.itemShortcuts.clear();

    for (int i = 0; i < SHORTCUT_ITEMS; i++)
    {
        if (mItems[i] >= 0)
            config.itemShortcuts.push_back({ i, mItems[i] });
    }
}

void ItemShortcut::useItem(int index)
{
    if (!mItems[index])
        return;

    Item *item = PlayerInfo::getInventory()->findItem(mItems[index]);
    if (item && item->getQuantity())
    {
        if (item->isEquippable())
        {
            if (item->isEquipped())
                item->doEvent(Event::DoUnequip);
            else
                item->doEvent(Event::DoEquip);
        }
        else
        {
            item->doEvent(Event::DoUse);
        }
    }
}
