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

#include <stdio.h>

#include "astar.h"
#include "being.h"

NODE *player_node = NULL;

std::list<NODE*> beings;


PATH_NODE::PATH_NODE(unsigned short x, unsigned short y):
    next(NULL)
{
    this->x = x;
    this->y = y;
}

PATH_NODE *calculate_path(
        unsigned short src_x, unsigned short src_y,
        unsigned short dest_x, unsigned short dest_y)
{
    return find_path(1, src_x, src_y, dest_x, dest_y);
}

NODE::NODE():
    id(0), job(0),
    action(0), frame(0),
    path(NULL),
    speech(NULL), speech_time(0),
    tick_time(0), speed(150),
    emotion(0), emotion_time(0),
    text_x(0), text_y(0),
    hair_style(1), hair_color(1),
    weapon(0)
{
    memset(coordinates, 0, 3);
    speech_color = makecol(0, 0, 0);
}

void empty() {
    std::list<NODE*>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        delete (*i);
    }
    beings.clear();
}

void add_node(NODE *node) {
    beings.push_back(node);
}

void remove_node(unsigned int id) {
    std::list<NODE*>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        if ((*i)->id == id) {
            delete (*i);
            beings.erase(i);
            return;
        }
    }
}

unsigned int find_npc(unsigned short x, unsigned short y) {
    std::list<NODE*>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        NODE *node = (*i);
        // Check if is a NPC (only low job ids)
        if (node->job >= 46 && node->job <= 125 &&
                get_x(node->coordinates) == x &&
                get_y(node->coordinates) == y)
        {
            return node->id;
        }
    }
    return 0;
}

unsigned int find_monster(unsigned short x, unsigned short y) {
    std::list<NODE*>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        NODE *node = (*i);
        // Check if is a MONSTER
        if (node->job > 200 &&
                get_x(node->coordinates) == x &&
                get_y(node->coordinates) == y)
        {
            return node->id;
        }
    }
    return 0;
}

NODE *find_node(unsigned int id) {
    std::list<NODE*>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        NODE *node = (*i);
        if (node->id == id) {
            return node;
        }
    }
    return NULL;
}

class NODE_Compare {
    public:
        bool operator() (const NODE *a, const NODE *b) const {
            return get_y(a->coordinates) < get_y(b->coordinates);
        }
};

void sort() {
    beings.sort(NODE_Compare());
}

void empty_path(NODE *node) {
    if (node) {
        PATH_NODE *temp = node->path;
        PATH_NODE *next;
        while (temp) {
            next = temp->next;
            delete temp;
            temp = next;
        }
        node->path = NULL;
    }
}

// Beings restructuration

Being::Being() {
    id = 0; job = 0;
    action = 0; frame = 0;
    path = NULL; speech = NULL; speech_time = 0;
    tick_time = 0; speed = 150;
    emotion = 0; emotion_time = 0;
    text_x = 0; text_y = 0;
    hair_style = 1; hair_color = 1;
    weapon = 0;
    x = 0; y = 0; direction = 0;
    speech_color = makecol(0, 0, 0);
}

Being::~Being() {
    if(path) {
        PATH_NODE *temp = path;
        PATH_NODE *next;
        while (temp) {
            next = temp->next;
            delete temp;
            temp = next;
        }
        path = NULL;        
    }
    if(speech) {
        free(speech);
    }    
}