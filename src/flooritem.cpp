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

#include "flooritem.h"

#include "net/net.h"
#include "net/playerhandler.h"

#include "resources/itemdb.h"
#include "resources/iteminfo.h"

FloorItem::FloorItem(int id,
                     int itemId,
                     const Vector &position,
                     Map *map):
    ActorSprite(id),
    mItemId(itemId),
    mX(0), mY(0)
{
    mPos = position;

    setMap(map);

    mX = (int)position.x / map->getTileWidth();
    mY = (int)position.y / map->getTileHeight();

    setupSpriteDisplay(itemDb->get(itemId).getDisplay());
}

const ItemInfo &FloorItem::getInfo() const
{
    return itemDb->get(mItemId);
}
