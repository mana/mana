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
 *  By ElvenProgrammer aka Eugenio Favalli (umperio@users.sourceforge.net)
 */

#ifndef _SHOP_H
#define _SHOP_H

#include <stdlib.h>
#include <stdio.h>

#include "../graphic/graphic.h"
#include "inventory.h"


struct ITEM_SHOP {
	char name[30];
	int price;
	short id;
	int index;
	int quantity;
	ITEM_SHOP *next;
};

extern int n_items;
extern char* selectedItem;
extern char itemCurrenyQ[10];

char *shop_list(int index, int *list_size);
void add_buy_item(short id, int price);
void add_sell_item(short index, int price);
void changeQ(void *dp3, int d2);
void close_shop();
short get_item_id(int index);
int get_item_quantity(int index);
int get_item_index(int index);
int get_item_price(int index);

#endif
