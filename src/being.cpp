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
#include "Being.h"

Being *player_node = NULL;

std::list<Being *> beings;


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

/*Being::Being():
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
}*/

void empty() {
    std::list<Being *>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        delete (*i);
    }
    beings.clear();
}

void add_node(Being *Being) {
    beings.push_back(Being);
}

void remove_node(unsigned int id) {
    std::list<Being *>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        if ((*i)->id == id) {
            delete (*i);
            beings.erase(i);
            return;
        }
    }
}

unsigned int find_npc(unsigned short x, unsigned short y) {
    std::list<Being *>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        Being *being = (*i);
        // Check if is a NPC (only low job ids)
        if (being->job >= 46 && being->job <= 125 &&
                being->x == x && being->y == y)
        {
            return being->id;
        }
    }
    return 0;
}

unsigned int find_monster(unsigned short x, unsigned short y) {
    std::list<Being*>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        Being *being = (*i);
        // Check if is a MONSTER
        if (being->job > 200 &&
                being->x == x &&
                being->y == y)
        {
            return being->id;
        }
    }
    return 0;
}

Being *find_node(unsigned int id) {
    std::list<Being*>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        Being *Being = (*i);
        if (Being->id == id) {
            return Being;
        }
    }
    return NULL;
}

class NODE_Compare {
    public:
        bool operator() (const Being *a, const Being *b) const {
            return a->y < b->y;
        }
};

void sort() {
    beings.sort(NODE_Compare());
}

void empty_path(Being *Being) {
    if (Being) {
        PATH_NODE *temp = Being->path;
        PATH_NODE *next;
        while (temp) {
            next = temp->next;
            delete temp;
            temp = next;
        }
        Being->path = NULL;
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
    if (path) {
        PATH_NODE *temp = path;
        PATH_NODE *next;
        while (temp) {
            next = temp->next;
            delete temp;
            temp = next;
        }
        path = NULL;        
    }
    if (speech) {
        free(speech);
    }    
}
