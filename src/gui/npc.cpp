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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "npc.h"

ITEM *item_head = NULL;
int item_number = 0;

char *item_list(int index, int *list_size) {
  if(index<0) {
    *list_size = item_number;
		return NULL;
	} else {
		int iterator = 0;
		ITEM *temp = item_head;
		while(iterator<index) {
			temp = temp->next;
			iterator++;
		}
		return temp->name;
	}
}

void add_item(char *name) {
	ITEM *item = item_head;
	ITEM *temp = (ITEM *)malloc(sizeof(ITEM));
	//temp->name = NULL;
	temp->name = name;
	temp->next = NULL;
	if(!item_head)
		item_head = temp;
	else {
		while(item->next)
			item = item->next;
		item->next = temp;
	}
	item_number++;
}

void remove_tail() {
  int iterator = 0;
  ITEM *temp = item_head;
  while(iterator<item_number-2) {
    temp = temp->next;
    iterator++;
  }
  free(temp->next);
  temp->next = NULL;
  item_number--;
}  

void parse_items(char *string, short len) {
  char *token = strtok(string, ":");
  while(token!=NULL) {
    //if(strcmp(token, "you prefer?")!=0) { // temp fix for the barber script
      char *temp = (char *)malloc(strlen(token));
      strcpy(temp, token);
      add_item(temp);
    //}  
    token = strtok(NULL, ":");
  }
  remove_tail();  
} 

void remove_all_items() {
  ITEM *temp, *next;
	temp = item_head;
	while(temp) {
		next = temp->next;
		/*if(temp->name!=NULL)
      free(temp->name);*/ //buggy
		free(temp);
		temp = next;
	}
	item_head = NULL;
	item_number = 0;
}

