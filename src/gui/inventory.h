/**

	The Mana World
	Copyright 2004 The Mana World Development Team

    This file is part of The Mana World.

    The Mana World is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    The Mana World is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Mana World; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

		By SimEdw

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
		TmwInventory() {};
		~TmwInventory() {};

		void create(int x, int y); // create the window
		void draw(BITMAP *); // draw the window (if areDisplaying != 0 )
		void show(bool val); // choose between showing and not showing the window
		void toggle() {if(show_inventory){show(0);}else{show(1);}}
		//API
		int add_item(int index, int id, int quantity); // add a item
		int remove_item(int id); // remove a item
		int change_quantity(int index, int quantity); // change number of a item
		int increase_quantity(int index, int quantity); // increase quantity of a item
		int use_item(int index, int id);
		int quantityForIndex(int index) { return items[index].quantity; }
		//END API

		itemHolder items[INVENTORY_SIZE]; // this is the holder of items
  private:
		DATAFILE *itemset;
		bool show_inventory;
};

#endif
