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

#include "inventory.h"

DIALOG inventory_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,   300,  252,  322,   60,    0,  -1,    0,    0,          0,                      0,    (char*)"Inventory",         NULL, NULL  },
   { NULL,                0,    0,   0,     0,    0,   0,    0,    0,          0,                      0,              NULL,         NULL, NULL  },
};

DIALOG_PLAYER *inventory_player;

/** Initialize inventory */
void TmwInventory::create(int tempxpos, int tempypos) {
	itemset = load_datafile("./data/graphic/items.dat");
	
	for(int i=0;i<INVENTORY_SIZE;i++) {
		items[i].id = -1; // if id is negative there's no item
		items[i].quantity = 0;
	}
	
	inventory_player = init_dialog(inventory_dialog, -1);
	position_dialog(inventory_dialog, x, y);

	show_inventory = false;
}

/** Draw inventory window */
void TmwInventory::draw(BITMAP * buffer) {
	if(show_inventory) {
		dialog_message(inventory_dialog,MSG_DRAW,0,0);
		update_dialog(inventory_player);		
	}
}

/** Set if inventory is visible */
void TmwInventory::show(bool val) {
	show_inventory = val;
}

/** Add an item the inventory */
int TmwInventory::add_item(int index, int id, int quantity) {
	items[index].id = id;
	items[index].quantity = quantity;
	return 0;
}

/** Remove a item from the inventory */
int TmwInventory::remove_item(int id) {
	for(int i=0;i<INVENTORY_SIZE;i++)
		if(items[i].id==id) {
			items[i].id = -1;
			items[i].quantity = 0;
		}  
	return 0;
}

/** Change quantity of an item */
int TmwInventory::change_quantity(int index, int quantity) {
  items[index].quantity = quantity;
  return 0;
}

int TmwInventory::useItem(int idnum) {
	printf("Use item %i\n",idnum);
	WFIFOW(0) = net_w_value(0x00a7);
	WFIFOW(2) = net_w_value(idnum);
	WFIFOSET(4);
	while((out_size>0))flush();

	return 0;
}*/
