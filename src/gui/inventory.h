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
 */

#ifdef WIN32
  #pragma warning (disable:4312)
#endif

#ifndef _INVENTORY_H
#define _INVENTORY_H 

#include <allegro.h>
#include "../main.h"
#include "../game.h"
#include "../log.h"
#include "../net/network.h"
#include "gui.h"

#define INVENTORY_SIZE 10

struct itemHolder { // the holder of a item
	int id;       // the id of the item
	int quantity; // number of items
	//int index;    // item position
};

class TmwInventory {
    public:
        TmwInventory();
        ~TmwInventory();

        /**
         * Initialize inventory and create the window.
         */
        void create(int x, int y);

        /**
         * Draw inventory window.
         */
        void draw(BITMAP *);

        /**
         * Sets if inventory is visible.
         */
        void setVisible(bool visible);

        /**
         * Returns visibility of inventory.
         */
        bool isVisible();

        /**
         * Add an item the inventory.
         */
        int add_item(int index, int id, int quantity);

        /**
         * Remove a item from the inventory.
         */
        int remove_item(int id);

        /**
         * Change quantity of an item.
         */
        int change_quantity(int index, int quantity);

        /**
         * Increase quantity of an item 
         */
        int increase_quantity(int index, int quantity);

        int use_item(int index, int id);
        int quantityForIndex(int index) { return items[index].quantity; }
        int drop_item(int index, int amunt);

        itemHolder items[INVENTORY_SIZE]; /**< this is the holder of items */
    private:
        DATAFILE *itemset;
        bool show_inventory;
        int itemMeny, itemMeny_x, itemMeny_y, itemMeny_i;
};

#endif
