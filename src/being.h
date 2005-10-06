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
#include <SDL_types.h>

#define NR_HAIR_STYLES 5
#define NR_HAIR_COLORS 10

class Map;
class Graphics;

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

        enum Direction {
            DIR_NONE  = -1,
            SOUTH     =  0,
            SW        =  1,
            WEST      =  2,
            NW        =  3,
            NORTH     =  4,
            NE        =  5,
            EAST      =  6,
            SE        =  7,
        };

        Uint16 job;           /**< Job (player job, npc, monster, ) */
        Uint16 x, y;          /**< Tile coordinates */
        Uint8 direction;      /**< Facing direction */
        Uint8 action;
        Uint8 frame;
        Sint32 speech_color;
        Uint16 walk_time;
        Uint16 speed;
        Uint8 emotion;        /**< Currently showing emotion */
        Uint8 emotion_time;   /**< Time until emotion disappears */
        Uint32 text_x, text_y;  // temp solution to fix speech position

        Uint16 aspd;          /**< Attack speed */

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
        void setDestination(Uint16 destX, Uint16 destY);

        /**
         * Puts a "speech balloon" above this being for the specified amount
         * of time.
         *
         * @param text The text that should appear.
         * @param time The amount of time the text should stay in milliseconds.
         */
        void setSpeech(const std::string &text, Uint32 time);

        /**
         * Puts a damage bubble above this being for the specified amount
         * of time.
         *
         * @param text The text that should appear.
         * @param time The amount of time the text should stay in milliseconds.
         */
        void setDamage(Sint16 amount, Uint32 time);

        /**
         * Returns the name of the being.
         */
        const std::string&
        getName() { return mName; }

        /**
         * Sets the name for the being.
         *
         * @param text The name that should appear.
         */
        void
        setName(const std::string &name) { mName = name; }

        /**
         * Sets the hair color for this being.
         */
        void setHairColor(Uint16 color);

        /**
         * Sets the hair style for this being.
         */
        void setHairStyle(Uint16 style);

        /**
         * Gets the hair color for this being.
         */
        Uint16 getHairColor();

        /**
         * Gets the hair style for this being.
         */
        Uint16 getHairStyle();

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
        Uint16  getWeapon() { return m_weapon; }

        /**
         * get the sprite id.
         */
        Uint32 getId() { return m_id; }

        // MODIFICATION METHODS

        /**
         * set the weapon picture id.
         *
         * @param weapon : the picture id
         */
        void setWeapon(Uint16 weapon);

        /**
         * set the weapon picture id with the weapon id.
         *
         * @param weapon : the weapon id
         */
        void setWeaponById(Uint16 weapon);

        /**
         * set the sprite id.
         */
        void setId(Uint32 id);

        /**
         * Set the map the being is on
         */
        void setMap(Map *map);

    private:
        Uint16 m_weapon;
        Uint32 m_id;              /**< Unique id */
        Map *map;

        std::list<PATH_NODE> path;
        std::string speech;
        std::string damage;
        Uint16 hairStyle, hairColor;
        Uint32 speech_time;
        Uint32 damage_time;
        bool showSpeech, showDamage;
        std::string mName;              /**< Name of character */

        /**
         * Sets the new path for this being.
         */
        void setPath(std::list<PATH_NODE> path);
};

/** Return a specific id Being */
Being *findNode(Uint32 id);

/** Return a being at specific coordinates */
Being *findNode(Uint16 x, Uint16 y);

/** Return a being at specific coordinates with specific type*/
Being *findNode(Uint16 x, Uint16 y, Being::Type type);

/** Create a being and add it to the list of beings */
Being *createBeing(Uint32 id, Uint16 job, Map *map);

/** Remove a Being */
void remove_node(Being *being);

/** Sort beings in vertical order */
void sort();

extern Being *player_node;

extern std::list<Being*> beings;

#endif
