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
 *  By SimEdw
 */

#include "inventory.h"

DIALOG inventory_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,   300,  252,  322,   60,    0,  -1,    0,    0,          0,                      0,    (char*)"Inventory",         NULL, NULL  },
   { NULL,                0,    0,   0,     0,    0,   0,    0,    0,          0,                      0,              NULL,         NULL, NULL  },
};

DIALOG_PLAYER *inventory_player;

TmwInventory::TmwInventory()
{
    show_inventory = false;
}

TmwInventory::~TmwInventory()
{
}

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
		for(int i=0;i<INVENTORY_SIZE;i++) {
			if(items[i].quantity>0) {
				if(items[i].id>=501 && items[i].id<=511)
					draw_rle_sprite(gui_bitmap, (RLE_SPRITE *)itemset[items[i].id-501].dat, inventory_dialog[0].x+24*i, inventory_dialog[0].y+26);
				//else
					//masked_blit((BITMAP *)itemset[0].dat, gui_bitmap, 0, 0, inventory_dialog[0].x+24*i, inventory_dialog[0].y+26, 22, 22);
				
				textprintf_ex(gui_bitmap, font, inventory_dialog[0].x+24*i, inventory_dialog[0].y+44, makecol(0,0,0), -1, "%i", items[i].quantity);
			}
		}
	}
	if(mouse_b & 2)
	{
		for(int i=0;i<INVENTORY_SIZE;i++) {
		if(items[i].quantity>0 && inventory_dialog[0].x+24*i+24 > mouse_x && inventory_dialog[0].x+24*i < mouse_x && inventory_dialog[0].y+44+24 > mouse_y && inventory_dialog[0].y+44 < mouse_y)
			{
			itemMeny = 1;
			itemMeny_x = 24*i;
			itemMeny_y = 44+24;
			itemMeny_i = i;
			}
		}


	}


	if(itemMeny){
	if(inventory_dialog[0].y+itemMeny_y < mouse_y && inventory_dialog[0].y+itemMeny_y+10 > mouse_y) {
		if(mouse_b&1)
			{
			use_item(itemMeny_i,items[itemMeny_i].id);
			itemMeny = 0;
			}
		textprintf_ex(buffer, font, inventory_dialog[0].x+itemMeny_x, inventory_dialog[0].y+itemMeny_y, makecol(255,237,33), -1, "Use item");
		} else {
		textprintf_ex(buffer, font, inventory_dialog[0].x+itemMeny_x, inventory_dialog[0].y+itemMeny_y, MAKECOL_BLACK, -1, "Use item");
		}
	if(inventory_dialog[0].y+itemMeny_y+10 < mouse_y && inventory_dialog[0].y+itemMeny_y+20 > mouse_y) {
		if(mouse_b&1)
			{
			drop_item(itemMeny_i,1);
			itemMeny = 0;
			}
		textprintf_ex(buffer, font, inventory_dialog[0].x+itemMeny_x, inventory_dialog[0].y+itemMeny_y+10, makecol(255,237,33), -1, "Del item");
		} else {
		textprintf_ex(buffer, font, inventory_dialog[0].x+itemMeny_x, inventory_dialog[0].y+itemMeny_y+10, MAKECOL_BLACK, -1, "Del item");
		}
	}

}


/** Set if inventory is visible */
void TmwInventory::show(bool val) {
	show_inventory = val;
}

/** Add an item the inventory */
int TmwInventory::add_item(int index, int id, int quantity) {
	items[index].id = id;
	items[index].quantity += quantity;
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

/** Increase quantity of an item */
int TmwInventory::increase_quantity(int index, int quantity) {
	items[index].quantity += quantity;
	return 0;
}

int TmwInventory::use_item(int index, int id) {
	WFIFOW(0) = net_w_value(0x00a7);
	WFIFOW(2) = net_w_value(index);
	WFIFOL(4) = net_l_value(id);
	// Note: id is dest of item, usually player_node->account_ID
	WFIFOSET(8);
	while((out_size>0))flush();
	return 0;
}

int TmwInventory::drop_item(int index, int amunt) {
	WFIFOW(0) = net_w_value(0x00a7);
	WFIFOW(2) = net_w_value(index);
	WFIFOL(4) = net_l_value(amunt);
	WFIFOSET(8);
	while((out_size>0))flush();
	return 0;
}

