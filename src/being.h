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
#include <vector>

#include "sprite.h"
#include "map.h"
#include "animatedsprite.h"

#define NR_HAIR_STYLES 8
#define NR_HAIR_COLORS 10

class AnimatedSprite;
class Equipment;
class Item;
class Map;
class Graphics;
class ImageSet;
class Particle;

/**
 * A position along a being's path.
 */
struct PATH_NODE
{
    /**
     * Constructor.
     */
    PATH_NODE(unsigned short x, unsigned short y):
        x(x), y(y)
    { }

    unsigned short x;
    unsigned short y;
};
typedef std::list<PATH_NODE> Path;
typedef Path::iterator PathIterator;

class Being : public Sprite
{
    public:
        enum Type {
            UNKNOWN,
            LOCALPLAYER,
            PLAYER,
            NPC,
            MONSTER
        };

        /**
         * Action the being is currently performing
         * WARNING: Has to be in sync with the same enum in the Being class
         * of the server!
         */
        enum Action {
            STAND,
            WALK,
            ATTACK,
            SIT,
            DEAD,
            HURT
        };

        enum Sprite {
            BASE_SPRITE = 0,
            SHOE_SPRITE,
            BOTTOMCLOTHES_SPRITE,
            TOPCLOTHES_SPRITE,
            HAIR_SPRITE,
            HAT_SPRITE,
            WEAPON_SPRITE,
            VECTOREND_SPRITE
        };

        enum TargetCursorSize {
            TC_SMALL = 0,
            TC_MEDIUM,
            TC_LARGE,
            NUM_TC
        };


        /**
         * Directions, to be used as bitmask values
         */
        enum { DOWN = 1, LEFT = 2, UP = 4, RIGHT = 8 };

        std::string mName;      /**< Name of character */
        Uint16 mJob;            /**< Job (player job, npc, monster, ) */
        Uint16 mX, mY;          /**< Pixel coordinates (tile center) */
        Action mAction;         /**< Action the being is performing */
        Uint16 mWalkTime;
        Uint8 mEmotion;         /**< Currently showing emotion */
        Uint8 mEmotionTime;     /**< Time until emotion disappears */

        Uint16 mAttackSpeed;          /**< Attack speed */

        /**
         * Constructor.
         */
        Being(Uint16 id, Uint16 job, Map *map);

        /**
         * Destructor.
         */
        virtual ~Being();

        /**
         * Removes all path nodes from this being.
         */
        void clearPath();

        /**
         * Sets a new destination for this being to walk to.
         */
        void setDestination(Uint16 destX, Uint16 destY);

        /**
         * Adjusts course to expected stat point.
         */
        void adjustCourse(Uint16, Uint16);

        /**
         * Adjusts course to expected start and end points.
         */
        void adjustCourse(Uint16, Uint16, Uint16, Uint16);

        /**
         * Puts a "speech balloon" above this being for the specified amount
         * of time.
         *
         * @param text The text that should appear.
         * @param time The amount of time the text should stay in milliseconds.
         */
        void setSpeech(const std::string &text, Uint32 time);

        /**
         * Puts a damage bubble above this being for the specified amount of
         * time.
         *
         * @param amount The amount of damage.
         */
        virtual void
        takeDamage(int amount);

        /**
         * Handles an attack of another being by this being.
         */
        virtual void
        handleAttack();

        /**
         * Returns the name of the being.
         */
        const std::string&
        getName() const { return mName; }

        /**
         * Sets the name for the being.
         *
         * @param name The name that should appear.
         */
        void
        setName(const std::string &name) { mName = name; }

        /**
         * Sets the hair color for this being.
         */
        virtual void
        setHairColor(Uint16 color);

        /**
         * Gets the hair color for this being.
         */
        Uint16
        getHairColor() const { return mHairColor; }

        /**
         * Sets the hair style for this being.
         */
        virtual void
        setHairStyle(Uint16 style);

        /**
         * Gets the hair style for this being.
         */
        Uint16
        getHairStyle() const { return mHairStyle; }

        /**
         * Sets visible equipments for this being.
         */
        virtual void
        setVisibleEquipment(Uint8 slot, int id);

        /**
         * Sets the sex for this being.
         */
        virtual void
        setSex(Uint8 sex) { mSex = sex; }

        /**
         * Gets the sex for this being.
         */
        Uint8
        getSex() const { return mSex; }

        /**
         * Makes this being take the next step of his path.
         */
        virtual void
        nextStep();

        /**
         * Performs being logic.
         */
        virtual void
        logic();

        /**
         * Draws the speech text above the being.
         */
        void
        drawSpeech(Graphics *graphics, int offsetX, int offsetY);

        /**
         * Draws the emotion picture above the being.
         */
        void
        drawEmotion(Graphics *graphics, int offsetX, int offsetY);

        /**
         * Draws the name text below the being.
         */
        virtual void
        drawName(Graphics *graphics, int offsetX, int offsetY) {};

        /**
         * Returns the type of the being.
         */
        virtual Type getType() const;

        /**
         * Gets the weapon picture id.
         */
        Uint16 getWeapon() const { return mWeapon; }

        /**
         * Sets the weapon picture id.
         *
         * @param weapon the picture id
         */
        virtual void
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
        Uint16
        getId() const { return mId; }

        /**
         * Sets the sprite id.
         */
        void
        setId(Uint16 id) { mId = id; }

        /**
         * Sets the map the being is on
         */
        void setMap(Map *map);

        /**
         * Sets the current action.
         */
        virtual void
        setAction(Action action);

        /**
         * Returns the direction the being is facing.
         */
        SpriteDirection getSpriteDirection() const
        { return SpriteDirection(mSpriteDirection); }

        /**
         * Sets the current direction.
         */
        void setDirection(Uint8 direction);

        /**
         * Draws this being to the given graphics context.
         *
         * @see Sprite::draw(Graphics, int, int)
         */
        virtual void
        draw(Graphics *graphics, int offsetX, int offsetY) const;

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
        getXOffset() const { return getOffset(mStepX); }

        /**
         * Get the current Y pixel offset.
         */
        int
        getYOffset() const { return getOffset(mStepY); }

        /**
         * Returns the horizontal size of the current base sprite of the being
         */
        virtual int
        getWidth() const;

        /**
         * Returns the vertical size of the current base sprite of the being
         */
        virtual int
        getHeight() const;

        /**
         * Returns the required size of a target cursor for this being
         */
        virtual Being::TargetCursorSize
        getTargetCursorSize() const
        { return TC_MEDIUM; }

        std::auto_ptr<Equipment> mEquipment;

        /**
         * Take control of a particle
         */
        void
        controlParticle(Particle *particle);

    protected:
        /**
         * Sets the new path for this being.
         */
        void
        setPath(const Path &path, int mod = 1024);

        Uint16 mId;                     /**< Unique being id */
        Uint8 mSex;                     /**< Character's gender */
        Uint16 mWeapon;                 /**< Weapon picture id */
        Uint16 mWalkSpeed;              /**< Walking speed */
        Uint16 mSpeedModifier;          /**< Modifier to keep course on sync (1024 = normal speed) */
        Uint8 mSpriteDirection;         /**< Facing direction */
        Uint8 mDirection;               /**< Walking direction */
        Map *mMap;                      /**< Map on which this being resides */
        SpriteIterator mSpriteIterator;

        Path mPath;
        std::string mSpeech;
        Uint16 mHairStyle, mHairColor;
        Uint32 mSpeechTime;
        Sint32 mPx, mPy;                /**< Pixel coordinates */

        std::vector<AnimatedSprite*> mSprites;
        std::vector<int> mEquipmentSpriteIDs;
        std::list<Particle *> mChildParticleEffects;

    private:
        int
        getOffset(int step) const;

        Sint16 mStepX, mStepY;
        Uint16 mStepTime;

        static int instances;           /**< Number of Being instances */
        static ImageSet *emotionSet;    /**< Emoticons used by beings */
};

#endif
