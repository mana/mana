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

#ifndef SIMPLEANIMAION_H
#define SIMPLEANIMAION_H

#include "utils/xml.h"

class Animation;
class Frame;
class Graphics;
class Image;

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
        SimpleAnimation(Animation *animation);

        /**
         * Creates a simple animation that creates its animation from XML Data.
         */
        SimpleAnimation(xmlNodePtr animationNode);

        ~SimpleAnimation();

        void setFrame(int frame);

        int getLength() const;

        void update(int timePassed);

        bool draw(Graphics* graphics, int posX, int posY) const;

        /**
         * Resets the animation.
         */
        void reset();

        Image *getCurrentImage() const;

    private:
        /** The hosted animation. */
        Animation *mAnimation;

        /** Time in game ticks the current frame is shown. */
        int mAnimationTime;

        /** Index of current animation phase. */
        int mAnimationPhase;

        /** Current animation phase. */
        Frame *mCurrentFrame;
};

#endif
