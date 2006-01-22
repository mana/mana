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

#include "floor_item.h"
#include "graphic/spriteset.h"
#include "resources/itemmanager.h"
#include "resources/iteminfo.h"

#include <list>

extern Spriteset *itemset;

typedef std::list<FloorItem*> FloorItems;
FloorItems floorItems;

FloorItem::FloorItem(unsigned int id,
                     unsigned int itemId,
                     unsigned short x,
                     unsigned short y,
                     Map *map):
    mId(id),
    mItemId(itemId),
    mX(x),
    mY(y),
    mMap(map)
{
    // Retrieve item image using a global itemset and itemDb (alternative?)
    mImage = itemset->spriteset[itemDb->getItemInfo(itemId)->getImage() - 1];

    // Add ourselves to the map
    mSpriteIterator = mMap->addSprite(this);
}

FloorItem::~FloorItem()
{
    // Remove and delete the representative sprite
    mMap->removeSprite(mSpriteIterator);
}

void empty_floor_items()
{
    FloorItems::iterator i;
    for (i = floorItems.begin(); i != floorItems.end(); i++) {
        delete (*i);
    }
    floorItems.clear();
}

void add_floor_item(FloorItem *floorItem)
{
    floorItems.push_back(floorItem);
}

void remove_floor_item(unsigned int id)
{
    FloorItems::iterator i;

    for (i = floorItems.begin(); i != floorItems.end(); i++)
    {
        if ((*i)->getId() == id)
        {
            delete (*i);
            floorItems.erase(i);
            return;
        }
    }
}

FloorItem* find_floor_item_by_cor(unsigned short x, unsigned short y)
{
    FloorItems::iterator i;

    for (i = floorItems.begin(); i != floorItems.end(); i++)
    {
        FloorItem *floorItem = (*i);

        if (floorItem->getX() == x && floorItem->getY() == y)
        {
            return floorItem;
        }
    }

    return 0;
}

FloorItem *find_floor_item_by_id(unsigned int int_id)
{
    FloorItems::iterator i;

    for (i = floorItems.begin(); i != floorItems.end(); i++)
    {
        FloorItem *floorItem = (*i);

        if (floorItem->getId() == int_id)
        {
            return floorItem;
        }
    }

    return NULL;
}
