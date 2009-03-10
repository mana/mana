/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef BEING_H
#define BEING_H

#include <guichan/color.hpp>

#include <SDL_types.h>

#include <set>
#include <string>
#include <vector>

#include "particlecontainer.h"
#include "position.h"
#include "sprite.h"

#include "resources/spritedef.h"

#define FIRST_IGNORE_EMOTE 14
#define STATUS_EFFECTS 32

#define SPEECH_TIME 500
#define SPEECH_MAX_TIME 1000

class AnimatedSprite;
class Image;
class ItemInfo;
class Item;
class Map;
class Graphics;
class Particle;
class Position;
class SimpleAnimation;
class SpeechBubble;
class Text;

class StatusEffect;

typedef std::list<Sprite*> Sprites;
typedef Sprites::iterator SpriteIterator;

enum Gender
{
    GENDER_MALE = 0,
    GENDER_FEMALE = 1,
    GENDER_UNSPECIFIED = 2
};

class Being : public Sprite
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
         * Action the being is currently performing.
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

        enum Sprite
        {
            BASE_SPRITE = 0,
            SHOE_SPRITE,
            BOTTOMCLOTHES_SPRITE,
            TOPCLOTHES_SPRITE,
            MISC1_SPRITE,
            MISC2_SPRITE,
            HAIR_SPRITE,
            HAT_SPRITE,
            CAPE_SPRITE,
            GLOVES_SPRITE,
            WEAPON_SPRITE,
            SHIELD_SPRITE,
            VECTOREND_SPRITE
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
            NAME_IN_BUBBLE,
            NUM_SPEECH
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

        Uint16 mJob;          /**< Job (player job, npc, monster, ) */
        Uint16 mX, mY;        /**< Tile coordinates */
        Action mAction;       /**< Action the being is performing */
        int mFrame;
        int mWalkTime;
        int mEmotion;         /**< Currently showing emotion */
        int mEmotionTime;     /**< Time until emotion disappears */
        int mSpeechTime;

        int mAttackSpeed;     /**< Attack speed */

        /**
         * Constructor.
         */
        Being(int id, int job, Map *map);

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
        virtual void setDestination(Uint16 destX, Uint16 destY);

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
        virtual void setName(const std::string &name)
        { mName = name; }

        /**
         * Gets the hair color for this being.
         */
        int getHairColor() const
        { return mHairColor; }

        /**
         * Gets the hair style for this being.
         */
        int getHairStyle() const
        { return mHairStyle; }

        /**
         * Get the number of hairstyles implemented
         */
        static int getNumOfHairstyles() { return mNumberOfHairstyles; }

        /**
         * Sets the hair style and color for this being.
         */
        virtual void setHairStyle(int style, int color);

        /**
         * Sets visible equipments for this being.
         */
        virtual void setSprite(int slot, int id, std::string color = "");

        /**
         * Sets the gender of this being.
         */
        virtual void setGender(Gender gender) { mGender = gender; }

        /**
         * Gets the gender of this being.
         */
        Gender getGender() const { return mGender; }

        /**
         * Makes this being take the next step of his path.
         */
        virtual void nextStep();

        /**
         * Triggers whether or not to show the name as a GM name.
         * NOTE: This doesn't mean that just anyone can use this.
         * If the server doesn't acknowlege you, you won't be shown
         * as a GM on other people's clients.
         */
        virtual void setGM() { mIsGM = true; }

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
        virtual Type getType() const;

        /**
         * Gets the walk speed.
         */
        Uint16 getWalkSpeed() const { return mWalkSpeed; }

        /**
         * Sets the walk speed.
         */
        void setWalkSpeed(Uint16 speed) { mWalkSpeed = speed; }

        /**
         * Gets the sprite id.
         */
        int getId() const { return mId; }

        /**
         * Sets the sprite id.
         */
        void setId(int id) { mId = id; }

        /**
         * Sets the map the being is on
         */
        void setMap(Map *map);

        /**
         * Sets the current action.
         */
        virtual void setAction(Action action);

        /**
         * Returns the current direction.
         */
        Uint8 getDirection() const { return mDirection; }

        /**
         * Sets the current direction.
         */
        void setDirection(Uint8 direction);

        /**
         * Gets the current action.
         */
        int getWalkTime() { return mWalkTime; }

        /**
         * Returns the direction the being is facing.
         */
        SpriteDirection getSpriteDirection() const;

        /**
         * Draws this being to the given graphics context.
         *
         * @see Sprite::draw(Graphics, int, int)
         */
        virtual void draw(Graphics *graphics, int offsetX, int offsetY) const;

        /**
         * Returns the pixel X coordinate.
         */
        int getPixelX() const { return mPx; }

        /**
         * Returns the pixel Y coordinate.
         *
         * @see Sprite::getPixelY()
         */
        int getPixelY() const { return mPy; }

        /**
         * Get the current X pixel offset.
         */
        int getXOffset() const
        { return getOffset(LEFT, RIGHT); }

        /**
         * Get the current Y pixel offset.
         */
        int getYOffset() const
        { return getOffset(UP, DOWN); }

        /**
         * Returns the horizontal size of the current base sprite of the being
         */
        virtual int getWidth() const;

        /**
         * Returns the vertical size of the current base sprite of the being
         */
        virtual int getHeight() const;

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
         * Sets the target animation for this being.
         */
        void setTargetAnimation(SimpleAnimation* animation);

        /**
         * Untargets the being
         */
        void untarget() { mUsedTargetCursor = NULL; }

        AnimatedSprite* getEmote(int index) { return emotionSet[index]; }

        void setEmote(Uint8 emotion, Uint8 emote_time)
        {
            mEmotion = emotion;
            mEmotionTime = emote_time;
        }

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

        // Target cursor being used by the being
        Image *mTargetCursor;

        static int getHairColorsNr();

        static int getHairStylesNr();

        static std::string getHairColor(int index);

        virtual AnimatedSprite* getSprite(int index) const
            { return mSprites[index]; }

    protected:
        /**
         * Sets the new path for this being.
         */
        void setPath(const Path &path);

        /**
         * Let the sub-classes react to a replacement
         */
        virtual void updateCoords() {}

        /**
         * Trigger visual effect, with components
         *
         * \param effectId ID of the effect to trigger
         * \param sfx Whether to trigger sound effects
         * \param gfx Whether to trigger graphical effects
         */
        void internalTriggerEffect(int effectId, bool sfx, bool gfx);

        /**
         * Notify self that the stun mode has been updated.  Invoked by
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

        int mId;                        /**< Unique sprite id */
        Uint16 mWalkSpeed;              /**< Walking speed */
        Uint8 mDirection;               /**< Facing direction */
        Map *mMap;                      /**< Map on which this being resides */
        std::string mName;              /**< Name of character */
        SpriteIterator mSpriteIterator;
        bool mIsGM;
        bool mParticleEffects;          /**< Whether to display particles or not */

        /** Engine-related infos about weapon. */
        const ItemInfo* mEquippedWeapon;

        static int mNumberOfHairstyles;          /** Number of hair styles in use */

        Path mPath;
        std::string mSpeech;
        Text *mText;
        Uint16 mHairStyle, mHairColor;
        Gender mGender;
        int mPx, mPy;                   /**< Pixel coordinates */
        Uint16 mStunMode;		/**< Stun mode; zero if not stunned */
        std::set<int> mStatusEffects;   /**< set of active status effects */

        gcn::Color mNameColor;

        std::vector<AnimatedSprite*> mSprites;
        std::vector<int> mSpriteIDs;
        std::vector<std::string> mSpriteColors;
        ParticleList mStunParticleEffects;
        ParticleVector mStatusParticleEffects;
        ParticleList mChildParticleEffects;

    private:
        /**
         * Calculates the offset in the given directions.
         * If walking in direction 'neg' the value is negated.
         */
        int getOffset(char pos, char neg) const;

        /** Reset particle status effects on next redraw? */
        bool mMustResetParticles;

        // Speech Bubble components
        SpeechBubble *mSpeechBubble;

        // Target cursor being used
        SimpleAnimation* mUsedTargetCursor;

        static int instances;                              /**< Number of Being instances */
        static std::vector<AnimatedSprite*> emotionSet;    /**< Emoticons used by beings */
};

#endif
