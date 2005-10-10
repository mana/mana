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

#include "sprite.h"
#include "map.h"

#define NR_HAIR_STYLES 5
#define NR_HAIR_COLORS 10

class Map;
class Graphics;

struct PATH_NODE
{
    /**
     * Constructor.
     */
    PATH_NODE(unsigned short x, unsigned short y);

    unsigned short x;
    unsigned short y;
};

class Being : public Sprite
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
            DEAD         = 15
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
            SE        =  7
        };

        Uint16 job;           /**< Job (player job, npc, monster, ) */
        Uint16 x, y;          /**< Tile coordinates */
        Uint8 direction;      /**< Facing direction */
        Uint8 action;
        Uint8 mFrame;
        Sint32 speech_color;
        Uint16 walk_time;
        Uint8 emotion;        /**< Currently showing emotion */
        Uint8 emotion_time;   /**< Time until emotion disappears */

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
        getName() const { return mName; }

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
        void
        setHairColor(Uint16 color);

        /**
         * Gets the hair color for this being.
         */
        Uint16
        getHairColor() const { return hairColor; }

        /**
         * Sets the hair style for this being.
         */
        void
        setHairStyle(Uint16 style);

        /**
         * Gets the hair style for this being.
         */
        Uint16
        getHairStyle() const { return hairStyle; }

        /**
         * Makes this being take the next step of his path.
         */
        void
        nextStep();

        /**
         * Performs being logic.
         */
        void
        logic();

        /**
         * Draws the speech text above the being.
         */
        void
        drawSpeech(Graphics *graphics, Sint32 offsetX, Sint32 offsetY);

        /**
         * Returns the type of the being.
         */
        Type getType() const;

        /**
         * Gets the weapon picture id.
         */
        Uint16 getWeapon() const { return mWeapon; }

        /**
         * Sets the weapon picture id.
         *
         * @param weapon the picture id
         */
        void
        setWeapon(Uint16 weapon) { mWeapon = weapon; }

        /**
         * Sets the weapon picture id with the weapon id.
         *
         * @param weapon the weapon id
         */
        void
        setWeaponById(Uint16 weapon);

        /**
         * Gets the walk speed.
         */
        Uint16
        getWalkSpeed() const { return mWalkSpeed; }

        /**
         * Sets the walk speed.
         */
        void
        setWalkSpeed(Uint16 speed) { mWalkSpeed = speed; }

        /**
         * Gets the sprite id.
         */
        Uint32
        getId() const { return mId; }

        /**
         * Sets the sprite id.
         */
        void
        setId(Uint32 id) { mId = id; }

        /**
         * Sets the map the being is on
         */
        void setMap(Map *map);

        /**
         * Draws this being to the given graphics context.
         *
         * @see Sprite::draw(Graphics, int, int)
         */
        void
        draw(Graphics *graphics, Sint32 offsetX, Sint32 offsetY);

        /**
         * Returns the pixel X coordinate.
         */
        int
        getPixelX() const { return mPx; }

        /**
         * Returns the pixel Y coordinate.
         *
         * @see Sprite::getPixelY()
         */
        int
        getPixelY() const { return mPy; }

        /**
         * Get the current X pixel offset.
         */
        int
        getXOffset() const;

        /**
         * Get the current Y pixel offset.
         */
        int
        getYOffset() const;

    private:
        /**
         * Sets the new path for this being.
         */
        void
        setPath(std::list<PATH_NODE> path);

        Uint32 mId;                     /**< Unique sprite id */
        Uint16 mWeapon;                 /**< Weapon picture id */
        Uint16 mWalkSpeed;              /**< Walking speed */
        Map *mMap;                      /**< Map on which this being resides */
        std::string mName;              /**< Name of character */
        Sprites::iterator mSpriteIterator;

        std::list<PATH_NODE> mPath;
        std::string speech;
        std::string damage;
        Uint16 hairStyle, hairColor;
        Uint32 speech_time;
        Uint32 damage_time;
        bool showSpeech, showDamage;
        Sint32 mPx, mPy;                /**< Pixel coordinates */
};

/**
 * Return a specific id Being
 */
Being*
findNode(Uint32 id);

/**
 * Return a being at specific coordinates
 */
Being*
findNode(Uint16 x, Uint16 y);

/**
 * Return a being at specific coordinates with specific type
 */
Being*
findNode(Uint16 x, Uint16 y, Being::Type type);

/**
 * Create a being and add it to the list of beings
 */
Being*
createBeing(Uint32 id, Uint16 job, Map *map);

/**
 * Remove a Being
 */
void
remove_node(Being *being);

extern Being *player_node;

typedef std::list<Being*> Beings;
extern Beings beings;

#endif
