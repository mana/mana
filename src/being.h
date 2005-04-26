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

class Being
{
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

        unsigned short weapon;
        char name[24];
        unsigned int speech_time;
        unsigned int damage_time;
        bool showSpeech, showDamage;
        unsigned short aspd; // attack speed

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
         * Sets a new destination for this being to walk to.
         */
        void setDestination(int x, int y);

        /**
         * Puts a "speech balloon" above this being for the specified amount
         * of time.
         *
         * @param text The text that should appear.
         * @param time The amount of time the text should stay in milliseconds.
         */
        void setSpeech(const std::string &text, int time);

        /**
         * Puts a damage bubble above this being for the specified amount
         * of time.
         *
         * @param text The text that should appear.
         * @param time The amount of time the text should stay in milliseconds.
         */
        void setDamage(const std::string &text, int time);

        /**
         * Sets the name for the being
         *
         * @param text The name that should appear.
         */
        void setName(char *name);

        /**
         * Sets the hair color for this being.
         */
        void setHairColor(int color);

        /**
         * Sets the hair style for this being.
         */
        void setHairStyle(int style);

        /**
         * Gets the hair color for this being.
         */
        unsigned short getHairColor();

        /**
         * Gets the hair style for this being.
         */
        unsigned short getHairStyle();

        /**
         * Makes this being take the next step of his path.
         */
        void nextStep();

        /**
         * Draws the speech text above the being.
         */
        void drawSpeech(Graphics *graphics);

        /**
         * Checks if the being is a player.
         */
        bool isPlayer();

        /**
         * Checks if the being is a npc.
         */
        bool isNpc();

        /**
         * Checks if the being is a monster.
         */
        bool isMonster();

    private:
        /**
         * Sets the new path for this being.
         */
        void setPath(std::list<PATH_NODE> path);

        std::list<PATH_NODE> path;
        std::string speech;
        std::string damage;
        unsigned short hairStyle, hairColor;
};

/** Add a Being to the list */
void add_node(Being *being);

/** Return a specific id Being */
Being *findNode(unsigned int id);

/** Return a being at specific coordinates */
Being *findNode(unsigned short x, unsigned short y);

/** Remove a Being */
void remove_node(unsigned int id);

/** Find a NPC id based on its coordinates */
unsigned int findNpc(unsigned short x, unsigned short y);

/** Find a PLAYER id based on its coordinates */
unsigned int findPlayer(unsigned short x, unsigned short y);

/** Find a MONSTER id based on its coordinates */
unsigned int findMonster(unsigned short x, unsigned short y);

/** Sort beings in vertical order */
void sort();

extern Being *player_node;

extern std::list<Being*> beings;

#endif
