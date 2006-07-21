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
#include <string>

#include "graphics.h"

class Image;
class Spriteset;

struct AnimationPhase
{
    int image;
    unsigned int delay;
    int offsetX;
    int offsetY;
};

class Animation
{
    public:
        Animation();

        void addPhase(int image, unsigned int delay, int offsetX, int offsetY);

        void update(unsigned int time);

        int getCurrentPhase();

        int getOffsetX() { return (*iCurrentPhase).offsetX; };
        int getOffsetY() { return (*iCurrentPhase).offsetY; };

        int getLength();

    protected:
        std::list<AnimationPhase> mAnimationPhases;
        std::list<AnimationPhase>::iterator iCurrentPhase;
        unsigned int mTime;
};

class Action
{
    public:
        Action();

        ~Action();

        void setImageset(std::string imageset) { mImageset = imageset; }

        std::string getImageset() { return mImageset; }

        void setAnimation(std::string direction, Animation *animation);

        Animation *getAnimation(std::string direction); // { return mAnimations[direction]; }

    protected:
        std::string mImageset;
        typedef std::map<std::string, Animation*> Animations;
        typedef Animations::iterator AnimationIterator;
        Animations mAnimations;
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
        AnimatedSprite(std::string animationFile, int variant);

        /**
         * Destructor.
         */
        ~AnimatedSprite();

        /**
         * Sets a new action using the current direction.
         */
        void play(std::string action);

        /**
         * Plays an action in a specified time.
         */
        void play(std::string action, int time);

        /**
         * Inform the animation of the passed time so that it can output the
         * correct animation phase.
         */
        void update(int time);

        /**
         * Draw the current animation phase at the coordinates given in screen
         * pixels
         */
        bool draw(Graphics * graphics, Sint32 posX, Sint32 posY);

        /**
         * Draw the current animation phase.
         */
        Image *getCurrentFrame();

        /**
         * gets the width in pixels of the current animation phase.
         */
        int getWidth();

        /**
         * gets the height in pixels of the current animation phase.
         */
        int getHeight();

        /**
         * Sets the direction.
         */
        void setDirection(std::string direction) { mDirection = direction; }

    protected:
        /**
         * When there are no animations defined for the action "complete", its
         * animations become a copy of those of the action "with".
         */
        void substituteAction(std::string complete, std::string with);

        typedef std::map<std::string, Spriteset*> Spritesets;
        typedef Spritesets::iterator SpritesetIterator;
        Spritesets mSpritesets;
        typedef std::map<std::string, Action*> Actions;
        Actions mActions;
        std::string mAction, mDirection;
        int mLastTime;
        float mSpeed;
};

#endif
