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

#include "map.h"

#include "resources/itemdb.h"
#include "resources/iteminfo.h"
#include "resources/spriteset.h"


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
    // Retrieve item image from item info
    mImage = ItemDB::get(itemId).getImage();

    // Add ourselves to the map
    mSpriteIterator = mMap->addSprite(this);
}

FloorItem::~FloorItem()
{
    // Remove ourselves from the map
    mMap->removeSprite(mSpriteIterator);
}
