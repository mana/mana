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

#include <list>
#include <string>
#include "graphics.h"

struct PATH_NODE {
    /**
     * Constructor.
     */
    PATH_NODE(unsigned short x, unsigned short y);

    unsigned short x, y;
};

class Being {
    private:
        std::list<PATH_NODE> path;
        std::string speech;
        unsigned char speech_time;

    public:
        unsigned int id;
        unsigned short job;
        unsigned short x, y;
        unsigned short destX, destY;
        unsigned char direction;
        unsigned char type;
        unsigned char action;
        unsigned char frame;
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
        void setPath(std::list<PATH_NODE> path);

        /**
         * Puts a "speech balloon" above this being for the specified amount
         * of time.
         *
         * @param text The text that should appear.
         * @param time The amount of time the text should stay in milliseconds.
         */
        void setSpeech(const std::string &text, int time);

        /**
         * Sets the hair color for this being.
         */
        void setHairColor(int color);

        /**
         * Sets the hair style for this being.
         */
        void setHairStyle(int style);

        /**
         * Makes this being take the next step of his path.
         */
        void nextStep();

        /**
         * Draws the speech text above the being.
         */
        void drawSpeech(Graphics *graphics);

        /**
         * Tick gives the being a sense of time. It should be called either a
         * specific amount of times per second, or be modified to be passed a
         * number that tells it the time since the last call.
         */
        void tick();
};

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
