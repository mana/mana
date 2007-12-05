/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
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

#include "itemshortcut.h"

#include "item.h"
#include "localplayer.h"
#include "configuration.h"

#include "utils/tostring.h"

ItemShortcut::ItemShortcut *itemShortcut;

ItemShortcut::ItemShortcut():
    mItemSelected(NULL)
{
    for (int i = 0; i < SHORTCUT_ITEMS; i++)
    {
        mItems[i] = NULL;
    }
}

ItemShortcut::~ItemShortcut()
{
    save();
}

void ItemShortcut::load()
{
    for (int i = 0; i < SHORTCUT_ITEMS; i++)
    {
        int itemId = (int) config.getValue("itemShortcut" + toString(i), -1);

        if (itemId != -1)
        {
            Item* item = player_node->searchForItem(itemId);
            if (item)
            {
                mItems[i] = item;
            }
        }
    }
}

void ItemShortcut::save()
{
    for (int i = 0; i < SHORTCUT_ITEMS; i++)
    {
        if (mItems[i])
        {
            config.setValue("shortcut" + toString(i), mItems[i]->getId());
        }
        else
        {
            config.setValue("shortcut" + toString(i), -1);
        }
    }
}

void ItemShortcut::useItem(int index)
{
    if (mItems[index])
    {
        if (mItems[index]->getQuantity()) {
            player_node->useItem(mItems[index]);
        }
    }
}
