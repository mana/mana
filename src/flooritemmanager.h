/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FLOORITEMMANAGER_H
#define FLOORITEMMANAGER_H

#include <list>

class FloorItem;
class Map;

class FloorItemManager
{
    public:
        ~FloorItemManager();

        FloorItem* create(int id, int itemId, int x, int y, Map *map);

        void destroy(FloorItem *item);

        void clear();

        FloorItem* findById(int id);
        FloorItem* findByCoordinates(int x, int y);

    private:
        typedef std::list<FloorItem*> FloorItems;
        typedef FloorItems::iterator FloorItemIterator;
        FloorItems mFloorItems;

};

// TODO Get rid of the global?
extern FloorItemManager *floorItemManager;

#endif
