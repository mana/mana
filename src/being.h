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

#ifndef _TMW_BEING_H
#define _TMW_BEING_H

#include "net/protocol.h"
#include <list>

struct PATH_NODE {
    /**
     * Constructor.
     */
    PATH_NODE(unsigned short x, unsigned short y);

    unsigned short x, y;
    PATH_NODE *next;
};

class Being {
    private:
        PATH_NODE *path;

    public:
        unsigned int id;
        unsigned short job;
        unsigned short x, y;
        unsigned char direction;
        unsigned char type;
        unsigned char action;
        unsigned char frame;
        char *speech;
        unsigned char speech_time;
        int speech_color;
        unsigned short walk_time;
        unsigned short speed;
        unsigned char emotion;
        unsigned char emotion_time;
        unsigned int text_x, text_y; // temp solution to fix speech position
        unsigned short hair_style, hair_color;
        unsigned short weapon;

        /**
         * Constructor.
         */
        Being();

        /**
         * Destructor.
         */
        ~Being();

        /**
         * Removes all path nodes from this being.
         */
        void clearPath();

        /**
         * Sets the new path for this being.
         */
        void setPath(PATH_NODE *path);

        /**
         * Returns wether this being has a path to follow.
         */
        bool hasPath();

        /**
         * Makes this being take the next step of his path.
         */
        void nextStep();
};

/** Removes all beings from the list */
void empty();

/** Add a Being to the list */
void add_node(Being *being);

/** Return a specific id Being */
Being *find_node(unsigned int id);

/** Remove a Being */
void remove_node(unsigned int id);

/** Find a NPC id based on its coordinates */
unsigned int find_npc(unsigned short x, unsigned short y);

/** Find a MONSTER id based on its coordinates */
unsigned int find_monster(unsigned short x, unsigned short y);

/** Sort beings in vertical order */
void sort();

extern Being *player_node;

extern std::list<Being*> beings;

#endif
