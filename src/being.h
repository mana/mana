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

*/

#ifndef _BEING_H
#define _BEING_H

#include "./net/protocol.h"

#define ACTION_NODE  0
#define PLAYER_NODE  1
#define NPC_NODE     2
#define MONSTER_NODE 3

struct PATH_NODE {
	unsigned short x, y;
	PATH_NODE *next;
};

struct NODE {
	unsigned int id;
	short job;
	char coordinates[3];
	NODE *next;
	unsigned char type;
	unsigned char action;
	unsigned char frame;
	PATH_NODE *path;
	char *speech;
	unsigned char speech_time;
	int speech_color;
	short tick_time;
	short speed;
	unsigned char emotion;
	unsigned char emotion_time;
	int text_x, text_y; // temp solution to fix speech position
};

void empty();
NODE *get_head();
NODE *create_node();
PATH_NODE *create_path_node(unsigned short x, unsigned short y);
void add_node(NODE *node);
NODE *find_node(unsigned int id);
void remove_node(unsigned int id);
unsigned int get_count();
bool remove_being(char *data);
void remove_being(unsigned int id);
int get_beings_size();
bool is_being(int id);
void popup_being(unsigned char type, char *data);
void move_being(char *data);
PATH_NODE *calculate_path(unsigned short src_x, unsigned short src_y, unsigned short dest_x, unsigned short dest_y);
unsigned int get_id(NODE *node);
unsigned int find_npc(unsigned short x, unsigned short y);
unsigned int find_monster(unsigned short x, unsigned short y);
void sort();

extern NODE *player_node;

#endif
