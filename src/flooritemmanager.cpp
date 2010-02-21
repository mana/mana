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

#include "flooritemmanager.h"
#include "flooritem.h"

#include "game.h"

#include "utils/dtor.h"

FloorItemManager::~FloorItemManager()
{
    clear();
}

FloorItem *FloorItemManager::create(int id, int itemId, int x, int y)
{
    Map *map = Game::instance()->getCurrentMap();
    FloorItem *floorItem = new FloorItem(id, itemId, x, y, map);
    mFloorItems.push_back(floorItem);
    return floorItem;
}

void FloorItemManager::destroy(FloorItem *item)
{
    mFloorItems.remove(item);
    delete item;
}

void FloorItemManager::clear()
{
    delete_all(mFloorItems);
    mFloorItems.clear();
}

FloorItem *FloorItemManager::findById(int id) const
{
    FloorItems::const_iterator i;
    for (i = mFloorItems.begin(); i != mFloorItems.end(); i++)
    {
        if ((*i)->getId() == id)
        {
            return *i;
        }
    }

    return NULL;
}

FloorItem *FloorItemManager::findByCoordinates(int x, int y) const
{
    FloorItems::const_iterator i;
    for (i = mFloorItems.begin(); i != mFloorItems.end(); i++)
    {
        if ((*i)->getX() == x && (*i)->getY() == y)
        {
            return *i;
        }
    }

    return NULL;
}
