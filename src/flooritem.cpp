/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "flooritem.h"

#include "net/net.h"

#include "resources/itemdb.h"
#include "resources/iteminfo.h"

FloorItem::FloorItem(int id,
                     int itemId,
                     int x,
                     int y,
                     Map *map):
    ActorSprite(id),
    mItemId(itemId),
    mX(x),
    mY(y)
{
    setMap(map);

    // TODO: Eventually, we probably should fix all sprite offsets so that
    //       these translations aren't necessary anymore. The sprites know
    //       best where their base point should be.
    mPos.x = x * map->getTileWidth() + 16;
    mPos.y = y * map->getTileHeight() + 16;

    setupSpriteDisplay(itemDb->get(itemId).getDisplay());
}

const ItemInfo &FloorItem::getInfo() const
{
    return itemDb->get(mItemId);
}
