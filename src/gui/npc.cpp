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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "npc.h"
#include <vector>

std::vector<ITEM*> items;


char *item_list(int index, int *list_size) {
    if (index < 0) {
        *list_size = items.size();
        return NULL;
    } else {
        return items[index]->name;
    }
}

void add_item(char *name) {
    ITEM *item = (ITEM*)malloc(sizeof(ITEM));
    item->name = name;
    items.push_back(item);
}

void remove_tail() {
    free(items.back()->name);
    free(items.back());
    items.pop_back();
}  

void parse_items(char *string, short len) {
    char *token = strtok(string, ":");
    while (token != NULL) {
        // temp fix for the barber script
        //if (strcmp(token, "you prefer?") != 0) {
        char *temp = (char*)malloc(strlen(token));
        strcpy(temp, token);
        add_item(temp);
        //}
        token = strtok(NULL, ":");
    }
    remove_tail();  
} 

void remove_all_items() {
    int i;
    for (i = 0; i < items.size(); i++) {
        free(items[i]->name);
        free(items[i]);
    }
    items.clear();
}
