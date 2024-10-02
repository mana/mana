/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#ifndef SIMPLEANIMAION_H
#define SIMPLEANIMAION_H

#include "resources/animation.h"

#include "utils/xml.h"

class Graphics;

/**
 * This class is a leightweight alternative to the AnimatedSprite class.
 * It hosts a looping animation without actions and directions.
 */
class SimpleAnimation final
{
    public:
        /**
         * Creates a simple animation with an already created \a animation.
         * Takes ownership over the given animation.
         */
        SimpleAnimation(Animation animation);

        /**
         * Creates a simple animation that creates its animation from XML Data.
         */
        SimpleAnimation(xmlNodePtr animationNode,
                        const std::string &dyePalettes = std::string());

        void setFrame(int frame);

        int getLength() const;

        void update(int dt);

        bool draw(Graphics *graphics, int posX, int posY) const;

        /**
         * Resets the animation.
         */
        void reset();

        Image *getCurrentImage() const;

    private:
        void initializeAnimation(xmlNodePtr animationNode,
                                 const std::string& dyePalettes = std::string());

        /** The hosted animation. */
        Animation mAnimation;

        /** Time in milliseconds the current frame is shown. */
        int mAnimationTime = 0;

        /** Index of current animation phase. */
        int mAnimationPhase = 0;

        /** Current animation phase. */
        Frame *mCurrentFrame = nullptr;

        /**  Tell whether the animation is ready */
        bool mInitialized = false;
};

#endif
