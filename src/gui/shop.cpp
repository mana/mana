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

#include "shop.h"
#include "../graphic/graphic.h"

int n_items;
char* selectedItem = "You got 4";
ITEM_SHOP *shop = NULL;

char *item_db[] = {
    "Cactus Drink", "Cactus potion", "Casino coins", "Decor Candy", "Maggot Slime",
    "Candy Cane", "Scorpion Stinger", "Xmas Cake", "Chocolate", "Candy", "Santa Hat"
};

char *shop_list(int index, int *list_size) {
    if(index<0) {
        *list_size = n_items;
        return NULL;
    } else {
        int iterator = 0;
        ITEM_SHOP *item_shop = shop;
        while(iterator<index) {
            item_shop = item_shop->next;
            iterator++;
        }
        return item_shop->name;
    }
}

void add_buy_item(short id, int price) {
    ITEM_SHOP *item_shop = (ITEM_SHOP *)malloc(sizeof(ITEM_SHOP));
    if(id-501>=0 && id-501<=11)
        sprintf(item_shop->name, "%s %i gp", item_db[id-501], price);
    else
        sprintf(item_shop->name, "Unknown item %i gp", price);
    item_shop->price = price;
    item_shop->id = id;
    item_shop->next = NULL;
    if(shop==NULL) 
        shop = item_shop;
    else {
        ITEM_SHOP *temp = shop;
        while(temp->next)
            temp = temp->next;
        temp->next = item_shop;
    }
}

void add_sell_item(short index, int price) {
    int id = inventoryWindow->items[index].id;
    ITEM_SHOP *item_shop = (ITEM_SHOP *)malloc(sizeof(ITEM_SHOP));
    if (id >= 501 && id <= 511)
        sprintf(item_shop->name, "%s %i gp", item_db[id-501], price);
    else
        sprintf(item_shop->name, "Unknown item %i gp", price);
    item_shop->price = price;
    item_shop->index = index;
    item_shop->id = id;
    item_shop->quantity  = inventoryWindow->items[index].quantity;
    item_shop->next = NULL;
    if (shop == NULL) 
        shop = item_shop;
    else {
        ITEM_SHOP *temp = shop;
        while(temp->next)
            temp = temp->next;
        temp->next = item_shop;
    }
}

void close_shop() {
    ITEM_SHOP *temp, *next;
    temp = shop;
    while(temp) {
        next = temp->next;
        free(temp);
        temp = next;
    }
    shop = NULL;
}

short get_item_id(int index) {
    int iterator = 0;
    ITEM_SHOP *item_shop = shop;
    while(iterator<index) {
        item_shop = item_shop->next;
        iterator++;
    }
    return item_shop->id;
}

int get_item_quantity(int index) {
    int iterator = 0;
    ITEM_SHOP *item_shop = shop;
    while(iterator<index) {
        item_shop = item_shop->next;
        iterator++;
    }
    if(item_shop)return item_shop->quantity;
    else return 0;
}

int get_item_index(int index) {
    int iterator = 0;
    ITEM_SHOP *item_shop = shop;
    while(iterator<index) {
        item_shop = item_shop->next;
        iterator++;
    }
    return item_shop->index;
}

int get_item_price(int index) {
    int iterator = 0;
    ITEM_SHOP *item_shop = shop;
    while(iterator<index) {
        item_shop = item_shop->next;
        iterator++;
    }
    return item_shop->price;
}

void changeQ(void *dp3, int d2) {
    sprintf(itemCurrenyQ,"%i",d2);
    printf("%s\n",itemCurrenyQ);
}
