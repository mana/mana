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

#include "astar.h"
#include "being.h"
#include "game.h"

Being *player_node = NULL;

std::list<Being*> beings;

PATH_NODE::PATH_NODE(unsigned short x, unsigned short y):
    next(NULL)
{
    this->x = x;
    this->y = y;
}

void empty() {
    std::list<Being *>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        delete (*i);
    }
    beings.clear();
}

void add_node(Being *being) {
    beings.push_back(being);
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
        Being *being = (*i);
        if (being->id == id) {
            return being;
        }
    }
    return NULL;
}

class BeingCompare {
    public:
        bool operator() (const Being *a, const Being *b) const {
            return a->y < b->y;
        }
};

void sort() {
    beings.sort(BeingCompare());
}

Being::Being() {
    id = 0; job = 0;
    action = 0; frame = 0;
    path = NULL; speech = NULL; speech_time = 0;
    walk_time = 0; speed = 150;
    emotion = 0; emotion_time = 0;
    text_x = 0; text_y = 0;
    hair_style = 1; hair_color = 1;
    weapon = 0;
    x = 0; y = 0; direction = 0;
    speech_color = 0;//makecol(0, 0, 0);
}

Being::~Being() {
    clearPath();
    if (speech) {
        free(speech);
    }    
}

void Being::clearPath() {
    PATH_NODE *temp = path;
    PATH_NODE *next;
    while (temp) {
        next = temp->next;
        delete temp;
        temp = next;
    }
    path = NULL;
}

void Being::setPath(PATH_NODE *path)
{
    clearPath();
    this->path = path;
    if (path != NULL) {
        direction = 0;
        if (path->next) {
            if (path->next->x > path->x && path->next->y > path->y)
                direction = SE;
            else if (path->next->x < path->x && path->next->y > path->y)
                direction = SW;
            else if (path->next->x > path->x && path->next->y < path->y)
                direction = NE;
            else if (path->next->x < path->x && path->next->y < path->y)
                direction = NW;
            else if (path->next->x > path->x)
                direction = EAST;
            else if (path->next->x < path->x)
                direction = WEST;
            else if (path->next->y > path->y)
                direction = SOUTH;
            else if (path->next->y < path->y)
                direction = NORTH;
        }
        PATH_NODE *pn = path;
        this->path = path->next;
        delete pn;
        x = this->path->x;
        y = this->path->y;
        action = WALK;
        walk_time = tick_time;
        frame = 0;
    }
}

bool Being::hasPath()
{
    return path != NULL;
}

void Being::nextStep()
{
    if (path->next) {
        int old_x, old_y, new_x, new_y;
        old_x = path->x;
        old_y = path->y;
        path = path->next;
        new_x = path->x;
        new_y = path->y;
        direction = 0;

        if (new_x > old_x) {
            if (new_y > old_y)      direction = SE;
            else if (new_y < old_y) direction = NE;
            else                    direction = EAST;
        }
        else if (new_x < old_x) {
            if (new_y > old_y)      direction = SW;
            else if (new_y < old_y) direction = NW;
            else                    direction = WEST;
        }
        else {
            if (new_y > old_y)      direction = SOUTH;
            else if (new_y < old_y) direction = NORTH;
        }

        x = path->x;
        y = path->y;
    } else {
        action = STAND;
    }
    frame = 0;
    walk_time = tick_time;
}
