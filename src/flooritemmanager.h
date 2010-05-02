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

#ifndef FLOORITEMMANAGER_H
#define FLOORITEMMANAGER_H

#include <list>

class FloorItem;
class Map;

class FloorItemManager
{
    public:
        ~FloorItemManager();

        FloorItem *create(int id, int itemId, int x, int y);

        void destroy(FloorItem *item);

        void clear();

        FloorItem *findById(int id) const;
        FloorItem *findByCoordinates(int x, int y) const;

        /**
         * Performs floor item logic.
         */
        void logic();

    private:
        typedef std::list<FloorItem*> FloorItems;
        typedef FloorItems::iterator FloorItemIterator;
        FloorItems mFloorItems;

};

// TODO Get rid of the global?
extern FloorItemManager *floorItemManager;

#endif
