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
#include "map.h"

#define NR_HAIR_STYLES 5
#define NR_HAIR_COLORS 10

class Map;

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
        enum Type {
            UNKNOWN,
            PLAYER,
            NPC,
            MONSTER
        };

        enum Action {
            STAND        =  0,
            WALK         =  1,
            ATTACK       =  5,
            BOW_ATTACK   =  9,
            MONSTER_DEAD =  9,
            SIT          = 13,
            HIT          = 14,
            DEAD         = 15,
        };

        unsigned short job;           /**< Job (player job, npc, monster, ) */
        unsigned short x, y;          /**< Tile coordinates */
        unsigned char direction;      /**< Facing direction */
        unsigned char action;
        unsigned char frame;
        int speech_color;
        unsigned short walk_time;
        unsigned short speed;
        unsigned char emotion;        /**< Currently showing emotion */
        unsigned char emotion_time;   /**< Time until emotion disappears */
        unsigned int text_x, text_y;  // temp solution to fix speech position

        char name[24];                /**< Name of character */
        unsigned short aspd;          /**< Attack speed */

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
        void setDestination(int destX, int destY);

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
         * Performs being logic.
         */
        void logic();

        /**
         * Draws the speech text above the being.
         */
        void drawSpeech(Graphics *graphics);

        /**
         * Returns the type of the being.
         */
        Type getType();

        // ACCES METHODS

        /**
         * get the weapon picture id.
         */
        unsigned short getWeapon() {return m_weapon;}

        /**
         * get the sprite id.
         */
        unsigned int getId() {return m_id;}

        // MODIFICATION METHODS

        /**
         * set the weapon picture id.
         *
         * @param weapon : the picture id
         */
        void setWeapon(unsigned short weapon);

        /**
         * set the weapon picture id with the weapon id.
         *
         * @param weapon : the weapon id
         */
        void setWeaponById(unsigned short weapon);

        /**
         * set the sprite id.
         */
        void setId(unsigned int id);

        /**
         * Set the map the being is on
         */
        void setMap(Map *map);

    private:
        unsigned short m_weapon;
        unsigned int m_id;              /**< Unique id */
        Map *map;

        std::list<PATH_NODE> path;
        std::string speech;
        std::string damage;
        unsigned short hairStyle, hairColor;
        unsigned int speech_time;
        unsigned int damage_time;
        bool showSpeech, showDamage;

        /**
         * Sets the new path for this being.
         */
        void setPath(std::list<PATH_NODE> path);
};

/** Add a Being to the list */
void add_node(Being *being);

/** Return a specific id Being */
Being *findNode(unsigned int id);

/** Return a being at specific coordinates */
Being *findNode(unsigned short x, unsigned short y);

/** Return a being at specific coordinates with specific type*/
Being *findNode(unsigned short x, unsigned short y, Being::Type type);

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
