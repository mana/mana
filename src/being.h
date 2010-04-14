/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BEING_H
#define BEING_H

#include "configlistener.h"
#include "map.h"
#include "particlecontainer.h"
#include "position.h"
#include "sprite.h"
#include "vector.h"

#include "resources/spritedef.h"

#include <guichan/color.hpp>

#include <SDL_types.h>

#include <set>
#include <string>
#include <vector>

#define FIRST_IGNORE_EMOTE 14
#define STATUS_EFFECTS 32

#define SPEECH_TIME 500
#define SPEECH_MAX_TIME 1000

class AnimatedSprite;
class FlashText;
class Graphics;
class Image;
class ItemInfo;
class Item;
class Particle;
class Position;
class SimpleAnimation;
class SpeechBubble;
class Text;

class StatusEffect;

class Being : public Sprite, public ConfigListener
{
    public:
        enum Type
        {
            UNKNOWN,
            PLAYER,
            NPC,
            MONSTER
        };

        /**
         * Action the being is currently performing
         * WARNING: Has to be in sync with the same enum in the Being class
         * of the server!
         */
        enum Action
        {
            STAND,
            WALK,
            ATTACK,
            SIT,
            DEAD,
            HURT
        };

        enum TargetCursorSize
        {
            TC_SMALL = 0,
            TC_MEDIUM,
            TC_LARGE,
            NUM_TC
        };

        enum Speech
        {
            NO_SPEECH = 0,
            TEXT_OVERHEAD,
            NO_NAME_IN_BUBBLE,
            NAME_IN_BUBBLE
        };

        enum AttackType
        {
            HIT = 0x00,
            CRITICAL = 0x0a,
            MULTI = 0x08,
            REFLECT = 0x04,
            FLEE = 0x0b
        };

        /**
         * Directions, to be used as bitmask values
         */
        enum { DOWN = 1, LEFT = 2, UP = 4, RIGHT = 8 };

        /**
         * Constructor.
         *
         * @param id      a unique being id
         * @param subtype partly determines the type of the being
         * @param map     the map the being is on
         */
        Being(int id, int subtype, Map *map);

        virtual ~Being();

        /**
         * Removes all path nodes from this being.
         */
        void clearPath();

        /**
         * Returns the walk time.
         * Used to know which frame to display and trigger
         * the next Tile step.
         * TODO: Used by eAthena only?
         */
        int getWalkTime() const { return mWalkTime; }

        /**
         * Set the current WalkTime value.
         * TODO: Used by eAthena only?
         * @see Ea::BeingHandler that set it to tick time.
         */
        void setWalkTime(int walkTime) { mWalkTime = walkTime; }

        /**
         * Makes this being take the next tile of its path.
         * TODO: Used by eAthena only?
         */
        virtual void nextTile();

        /**
         * Get the current X pixel offset.
         * TODO: Used by eAthena only?
         */
        int getXOffset() const
        { return getOffset(LEFT, RIGHT); }

        /**
         * Get the current Y pixel offset.
         * TODO: Used by eAthena only?
         */
        int getYOffset() const
        { return getOffset(UP, DOWN); }

        /**
         * Creates a path for the being from current position to ex and ey
         */
        void setDestination(int ex, int ey);

        /**
         * Returns the destination for this being.
         */
        const Vector &getDestination() const { return mDest; }

        /**
         * Returns the tile x coord
         */
        int getTileX() const
        { return mX; }

        /**
         * Returns the tile y coord
         */
        int getTileY() const
        { return mY; }

        /**
         * Sets the tile x and y coord
         */
        void setTileCoords(int x, int y)
        { mX = x; mY = y; }

        /**
         * Puts a "speech balloon" above this being for the specified amount
         * of time.
         *
         * @param text The text that should appear.
         * @param time The amount of time the text should stay in milliseconds.
         */
        void setSpeech(const std::string &text, int time = 500);

        /**
         * Puts a damage bubble above this being.
         *
         * @param attacker the attacking being
         * @param damage the amount of damage recieved (0 means miss)
         * @param type the attack type
         */
        virtual void takeDamage(Being *attacker, int damage, AttackType type);

        /**
         * Handles an attack of another being by this being.
         *
         * @param victim the victim being
         * @param damage the amount of damage dealt (0 means miss)
         * @param type the attack type
         */
        virtual void handleAttack(Being *victim, int damage, AttackType type);

        /**
         * Returns the name of the being.
         */
        const std::string &getName() const
        { return mName; }

        /**
         * Sets the name for the being.
         *
         * @param name The name that should appear.
         */
        virtual void setName(const std::string &name);

        bool getShowName() const
        { return mShowName; }

        virtual void setShowName(bool doShowName);

        /**
         * Following are set from the server (mainly for players)
         */
        void setPartyName(const std::string &name) { mPartyName = name; }

        const std::string &getPartyName() const { return mPartyName; }

        virtual void setGuildName(const std::string &name);

        virtual void setGuildPos(const std::string &pos);

        /**
         * Get the number of hairstyles implemented
         */
        static int getNumOfHairstyles()
        { return mNumberOfHairstyles; }

        /**
         * Get the number of layers used to draw the being
         */
        int getNumberOfLayers() const;

        /**
         * Performs being logic.
         */
        virtual void logic();

        /**
         * Draws the speech text above the being.
         */
        void drawSpeech(int offsetX, int offsetY);

        /**
         * Draws the emotion picture above the being.
         */
        void drawEmotion(Graphics *graphics, int offsetX, int offsetY);

        /**
         * Returns the type of the being.
         */
        virtual Type getType() const { return UNKNOWN; }

         /**
          * Return Being's current Job (player job, npc, monster, creature )
          */
        Uint16 getSubType() const { return mSubType; }

         /**
          * Set Being's current Job (player job, npc, monster, creature )
          */
        virtual void setSubtype(Uint16 subtype) { mSubType = subtype; }

        /**
         * Sets the walk speed.
         * in pixels per second for eAthena,
         * in tiles per second for Manaserv.
         */
        void setWalkSpeed(Vector speed) { mWalkSpeed = speed; }

        /**
         * Gets the walk speed.
         * in pixels per second for eAthena,
         * in tiles per second for Manaserv (0.1 precision).
         */
        Vector getWalkSpeed() const { return mWalkSpeed; }

        /**
         * Sets the attack speed.
         * @todo In what unit?
         */
        void setAttackSpeed(int speed) { mAttackSpeed = speed; }

        /**
         * Gets the attack speed.
         * @todo In what unit?
         */
        int getAttackSpeed() const { return mAttackSpeed; }

        /**
         * Sets the sprite id.
         */
        void setId(int id) { mId = id; }

        int getId() const { return mId; }

        /**
         * Sets the map the being is on
         */
        void setMap(Map *map);

        /**
         * Sets the current action.
         */
        virtual void setAction(Action action, int attackType = 0);

        /**
         * Get the being's action currently performed.
         */
        Action getCurrentAction() const { return mAction; }

        /**
         * Returns whether this being is still alive.
         */
        bool isAlive() const { return mAction != DEAD; }

        /**
         * Returns the current direction.
         */
        Uint8 getDirection() const { return mDirection; }

        /**
         * Sets the current direction.
         */
        void setDirection(Uint8 direction);

        /**
         * Returns the being's current sprite frame number.
         */
        int getCurrentFrame() const { return mFrame; }

        /**
         * Set the being's current sprite frame number.
         */
        void setFrame(int frame) { mFrame = frame; }

        /**
         * Returns the direction the being is facing.
         */
        SpriteDirection getSpriteDirection() const
        { return SpriteDirection(mSpriteDirection); }

        /**
         * Draws this being to the given graphics context.
         *
         * @see Sprite::draw(Graphics, int, int)
         *
         * TODO: The following two functions should be combined.
         * at some point draw(), was changed to use mPx and mPy, with arugements
         * only for the offset, drawSpriteAt() takes x, and y and draws the sprite
         * exactly at those coords (though it does do some computing to work how the
         * old draw() worked).
         */
        virtual void draw(Graphics *graphics, int offsetX, int offsetY) const;

        virtual void drawSpriteAt(Graphics *graphics, int x, int y) const;

        /**
         * Set the alpha opacity used to draw the being.
         */
        virtual void setAlpha(float alpha)
        { mAlpha = alpha; }

        /**
         * Returns the current alpha opacity of the Being.
         */
        virtual float getAlpha() const
        { return mAlpha; }

        /**
         * Returns the X coordinate in pixels.
         */
        int getPixelX() const
        { return (int) mPos.x; }

        /**
         * Returns the Y coordinate in pixels.
         *
         * @see Sprite::getPixelY()
         */
        int getPixelY() const
        { return (int) mPos.y; }

        /**
         * Sets the position of this being.
         */
        void setPosition(const Vector &pos);

        /**
         * Overloaded method provided for convenience.
         *
         * @see setPosition(const Vector &pos)
         */
        void setPosition(float x, float y, float z = 0.0f)
        {
            setPosition(Vector(x, y, z));
        }

        /**
         * Returns the position of this being.
         */
        const Vector &getPosition() const { return mPos; }

        /**
         * Returns the horizontal size of the current base sprite of the being.
         */
        virtual int getWidth() const;

        /**
         * Returns the vertical size of the current base sprite of the being.
         */
        virtual int getHeight() const;

        /**
         * Returns the being's pixel radius used to detect collisions.
         */
        virtual int getCollisionRadius() const;

        /**
         * Returns the required size of a target cursor for this being.
         */
        virtual Being::TargetCursorSize getTargetCursorSize() const
        { return TC_MEDIUM; }

        /**
         * Take control of a particle.
         */
        void controlParticle(Particle *particle);

        /**
         * Shoots a missile particle from this being, to target being
         */
        void fireMissile(Being *target, const std::string &particle);

        /**
         * Gets the way the object is blocked by other objects.
         */
        virtual unsigned char getWalkMask() const
        { return 0x00; } //can walk through everything

        /**
         * Returns the path this being is following. An empty path is returned
         * when this being isn't following any path currently.
         */
        const Path &getPath() const { return mPath; }

        /**
         * Sets the target animation for this being.
         */
        void setTargetAnimation(SimpleAnimation *animation);

        /**
         * Untargets the being
         */
        void untarget() { mUsedTargetCursor = NULL; }


        /**
         * Set the Emoticon type and time displayed above
         * the being.
         */
        void setEmote(Uint8 emotion, Uint8 emote_time)
        {
            mEmotion = emotion;
            mEmotionTime = emote_time;
        }

        /**
         * Get the current Emoticon type displayed above
         * the being.
         */
        Uint8 getEmotion() const { return mEmotion; }

        /**
         * Sets the being's stun mode.  If zero, the being is `normal',
         * otherwise it is `stunned' in some fashion.
         */
        void setStunMode(int stunMode)
        {
            if (mStunMode != stunMode)
                updateStunMode(mStunMode, stunMode);
            mStunMode = stunMode;
        };

        void setStatusEffect(int index, bool active);

        /**
         * A status effect block is a 16 bit mask of status effects.
         * We assign each such flag a block ID of offset + bitnr.
         *
         * These are NOT the same as the status effect indices.
         */
        void setStatusEffectBlock(int offset, Uint16 flags);

        /**
         * Triggers a visual effect, such as `level up'
         *
         * Only draws the visual effect, does not play sound effects
         *
         * \param effectId ID of the effect to trigger
         */
        virtual void triggerEffect(int effectId)
        {
            internalTriggerEffect(effectId, false, true);
        }

        virtual AnimatedSprite *getSprite(int index) const
        { return mSprites[index]; }

        static void load();

        virtual void optionChanged(const std::string &value) {}

        void flashName(int time);

        int getDamageTaken() const
        { return mDamageTaken; }

        void updateName();

    protected:
        /**
         * Sets the new path for this being.
         */
        void setPath(const Path &path);

        /**
         * Updates name's location.
         */
        virtual void updateCoords();

        /**
         * Gets the way the object blocks pathfinding for other objects
         */
        virtual Map::BlockType getBlockType() const
        { return Map::BLOCKTYPE_NONE; }

        /**
         * Trigger visual effect, with components
         *
         * \param effectId ID of the effect to trigger
         * \param sfx Whether to trigger sound effects
         * \param gfx Whether to trigger graphical effects
         */
        void internalTriggerEffect(int effectId, bool sfx, bool gfx);

        /**
         * Notify self that the stun mode has been updated. Invoked by
         * setStunMode if something changed.
         */
        virtual void updateStunMode(int oldMode, int newMode);

        /**
         * Notify self that a status effect has flipped.
         * The new flag is passed.
         */
        virtual void updateStatusEffect(int index, bool newStatus);

        /**
         * Handle an update to a status or stun effect
         *
         * \param The StatusEffect to effect
         * \param effectId -1 for stun, otherwise the effect index
         */
        virtual void handleStatusEffect(StatusEffect *effect, int effectId);

        virtual void showName();

        /** The current sprite Frame number to be displayed */
        int mFrame;

        /** Used to trigger the nextStep (walking on next Tile)
         * TODO: Used by eAthena only?
         */
        int mWalkTime;

        int mEmotion;         /**< Currently showing emotion */
        int mEmotionTime;     /**< Time until emotion disappears */
        /** Time until the last speech sentence disappears */
        int mSpeechTime;

        int mAttackSpeed;     /**< Attack speed */
        Action mAction;       /**< Action the being is performing */
        Uint16 mSubType;      /**< Subtype (graphical view, basically) */

        int mId;                        /**< Unique sprite id */
        Uint8 mDirection;               /**< Facing direction */
        Uint8 mSpriteDirection;         /**< Facing direction */
        Map *mMap;                      /**< Map on which this being resides */
        std::string mName;              /**< Name of character */
        std::string mPartyName;
        MapSprite mMapSprite;

        /**
         * Holds a text object when the being displays it's name, 0 otherwise
         */
        FlashText *mDispName;
        const gcn::Color *mNameColor;
        bool mShowName;

        /** Engine-related infos about weapon. */
        const ItemInfo *mEquippedWeapon;

        static int mNumberOfHairstyles;          /** Number of hair styles in use */

        Path mPath;
        std::string mSpeech;
        Text *mText;
        const gcn::Color *mTextColor;
        Uint16 mStunMode;               /**< Stun mode; zero if not stunned */
        std::set<int> mStatusEffects;   /**< set of active status effects */

        typedef std::vector<AnimatedSprite*> Sprites;
        typedef Sprites::iterator SpriteIterator;
        typedef Sprites::const_iterator SpriteConstIterator;
        Sprites mSprites;
        float mAlpha;                   /**< Alpha opacity to draw the sprite */

        ParticleList mStunParticleEffects;
        ParticleVector mStatusParticleEffects;
        ParticleList mChildParticleEffects;

        Vector mDest;  /**< destination coordinates. */

    private:

        /**
         * Calculates the offset in the given directions.
         * If walking in direction 'neg' the value is negated.
         * TODO: Used by eAthena only?
         */
        int getOffset(char pos, char neg) const;

        /** Reset particle status effects on next redraw? */
        bool mMustResetParticles;

        /** Speech Bubble components */
        SpeechBubble *mSpeechBubble;

        /**
         * Walk speed for x and y movement values.
         * In pixels per second for eAthena,
         * In pixels per ticks for Manaserv.
         * @see MILLISECONDS_IN_A_TICK
         */
        Vector mWalkSpeed;

        Vector mPos;  /**< Position coordinates. */
        int mX, mY;   /**< Position in tile */

        int mDamageTaken;

        /** Target cursor being used */
        SimpleAnimation *mUsedTargetCursor;
};

#endif
