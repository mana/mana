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

#ifndef _TMW_ANIMATION_H
#define _TMW_ANIMATION_H

#include <list>
#include <map>

#include <libxml/tree.h>

class Image;
class Spriteset;

/**
 * A single frame in an animation, with a delay and an offset.
 */
struct AnimationPhase
{
    int image;
    unsigned int delay;
    int offsetX;
    int offsetY;
};

/**
 * An animation consists of several frames, each with their own delay and
 * offset.
 */
class Animation
{
    public:
        /**
         * Constructor.
         */
        Animation();

        /**
         * Restarts the animation from the first frame.
         */
        void
        reset();

        void
        addPhase(int image, unsigned int delay, int offsetX, int offsetY);

        void
        update(unsigned int time);

        int
        getCurrentPhase() const;

        /**
         * Returns the x offset of the current frame.
         */
        int
        getOffsetX() const { return iCurrentPhase->offsetX; };

        /**
         * Returns the y offset of the current frame.
         */
        int
        getOffsetY() const { return iCurrentPhase->offsetY; };

        /**
         * Returns the length of this animation.
         */
        int
        getLength();

    protected:
        std::list<AnimationPhase> mAnimationPhases;
        std::list<AnimationPhase>::iterator iCurrentPhase;
        unsigned int mTime;
};

/**
 * An action consists of several animations, one for each direction.
 */
class Action
{
    public:
        /**
         * Constructor.
         */
        Action();

        /**
         * Destructor.
         */
        ~Action();

        /**
         * Sets the spriteset used by this action.
         */
        void
        setSpriteset(Spriteset *spriteset) { mSpriteset = spriteset; }

        /**
         * Returns the spriteset used by this action.
         */
        Spriteset*
        getSpriteset() const { return mSpriteset; }

        void
        setAnimation(int direction, Animation *animation);

        /**
         * Resets all animations associated with this action.
         */
        void
        reset();

        Animation*
        getAnimation(int direction) const;

    protected:
        Spriteset *mSpriteset;
        typedef std::map<int, Animation*> Animations;
        typedef Animations::iterator AnimationIterator;
        Animations mAnimations;
};

#endif
