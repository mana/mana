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

#ifndef _TMW_SIMPLEANIMAION_H
#define _TMW_SIMPLEANIMAION_H

#include "resources/animation.h"

#include "utils/xml.h"

class Frame;
class Graphics;

/**
 * This class is a leightweight alternative to the AnimatedSprite class.
 * It hosts a looping animation without actions and directions.
 */
class SimpleAnimation
{
    public:
        /**
         * Creates a simple animation with an already created animation.
         */
        SimpleAnimation(Animation *animation):
            mAnimation(animation),
            mAnimationTime(0),
            mAnimationPhase(0),
            mCurrentFrame(mAnimation->getFrame(0))
        {};

        /**
         * Creates a simple animation that creates its animation from XML Data.
         */
        SimpleAnimation(xmlNodePtr animationNode);

        ~SimpleAnimation();

        void update(unsigned int timePassed);

        Image *getCurrentImage() const;

    private:
        /** The hosted animation. */
        Animation *mAnimation;

        /** Time in game ticks the current frame is shown. */
        unsigned int mAnimationTime;

        /** Index of current animation phase. */
        unsigned int mAnimationPhase;

        /** Current animation phase. */
        Frame *mCurrentFrame;
};

#endif
