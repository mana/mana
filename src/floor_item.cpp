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
#include "game.h"

std::list<FloorItem*> floorItems;

void empty_floor_items() {
    std::list<FloorItem *>::iterator i;
    for (i = floorItems.begin(); i != floorItems.end(); i++) {
        delete (*i);
    }
    floorItems.clear();
}

void add_floor_item(FloorItem *floorItem) {
    floorItems.push_back(floorItem);
}

void remove_floor_item(unsigned int int_id) {
    std::list<FloorItem *>::iterator i;
    for (i = floorItems.begin(); i != floorItems.end(); i++) {
        if ((*i)->int_id == int_id) {
            delete (*i);
            floorItems.erase(i);
            return;
        }
    }
}

unsigned int find_floor_item_by_cor(unsigned short x, unsigned short y) {
    std::list<FloorItem *>::iterator i;
    for (i = floorItems.begin(); i != floorItems.end(); i++) {
        FloorItem *floorItem = (*i);
        if (floorItem->x == x && floorItem->y == y)
        {
            return floorItem->int_id;
        }
    }
    return 0;
}

FloorItem *find_floor_item_by_id(unsigned int int_id) {
    std::list<FloorItem*>::iterator i;
    for (i = floorItems.begin(); i != floorItems.end(); i++) {
        FloorItem *floorItem = (*i);
        if (floorItem->int_id == int_id) {
            return floorItem;
        }
    }
    return NULL;
}

FloorItem::FloorItem() {
    id = 0; int_id = 0;
    x = 0; y = 0;
}

FloorItem::~FloorItem() { 
}
