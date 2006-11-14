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

#ifndef _TMW_ANIMATEDSPRITE_H
#define _TMW_ANIMATEDSPRITE_H

#include <map>
#include <string>
#include <SDL_types.h>

#include <libxml/tree.h>

class Action;
class Graphics;
class Spriteset;
struct AnimationPhase;

enum SpriteAction
{
    ACTION_DEFAULT = 0,
    ACTION_STAND,
    ACTION_WALK,
    ACTION_RUN,
    ACTION_ATTACK,
    ACTION_ATTACK_SWING,
    ACTION_ATTACK_STAB,
    ACTION_ATTACK_BOW,
    ACTION_ATTACK_THROW,
    ACTION_CAST_MAGIC,
    ACTION_USE_ITEM,
    ACTION_SIT,
    ACTION_SLEEP,
    ACTION_HURT,
    ACTION_DEAD,
    ACTION_INVALID
};

enum SpriteDirection
{
    DIRECTION_DEFAULT = 0,
    DIRECTION_DOWN,
    DIRECTION_UP,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_INVALID
};

/**
 * Defines a class to load an animation.
 */
class AnimatedSprite
{
    public:
        /**
         * Constructor.
         */
        AnimatedSprite(const std::string& animationFile, int variant);

        /**
         * Destructor.
         */
        ~AnimatedSprite();

        /**
         * Resets the animated sprite. This is used to synchronize several
         * animated sprites.
         */
        void
        reset();

        /**
         * Plays an action using the current direction
         */
        void
        play(SpriteAction action);

        /**
         * Inform the animation of the passed time so that it can output the
         * correct animation phase.
         */
        void
        update(int time);

        /**
         * Draw the current animation phase at the coordinates given in screen
         * pixels.
         */
        bool
        draw(Graphics* graphics, Sint32 posX, Sint32 posY) const;

        /**
         * Returns the width in pixels of the current animation phase.
         */
        int
        getWidth() const;

        /**
         * Returns the height in pixels of the current animation phase.
         */
        int
        getHeight() const;

        /**
         * Sets the direction.
         */
        void
        setDirection(SpriteDirection direction)
        {
            mDirection = direction;
        }

    private:
        /**
         * When there are no animations defined for the action "complete", its
         * animations become a copy of those of the action "with".
         */
        void
        substituteAction(SpriteAction complete, SpriteAction with);

        /**
         * Returns the current animation frame.
         */
        const AnimationPhase*
        getCurrentPhase() const;

        /**
         * Gets an integer property from an xmlNodePtr.
         *
         * TODO: Same function is present in MapReader. Should probably be
         * TODO: shared in a static utility class.
         */
        static int
        getProperty(xmlNodePtr node, const char *name, int def);

        /**
         * Gets a string property from an xmlNodePtr.
         */
        static std::string
        getProperty(xmlNodePtr node, const char *name, const std::string &def);

        /**
         * Converts a string into a SpriteAction enum.
         */
        static SpriteAction
        makeSpriteAction(const std::string &action);

        /**
         * Converts a string into a SpriteDirection enum.
         */
        static SpriteDirection
        makeSpriteDirection(const std::string &direction);


        typedef std::map<std::string, Spriteset*> Spritesets;
        typedef Spritesets::iterator SpritesetIterator;

        typedef std::map<SpriteAction, Action*> Actions;
        typedef Actions::iterator ActionIterator;

        Spritesets mSpritesets;
        Actions mActions;
        Action *mAction;
        SpriteDirection mDirection;
        int mLastTime;
        float mSpeed;
};

#endif
