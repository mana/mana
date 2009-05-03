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

#ifndef ANIMATEDSPRITE_H
#define ANIMATEDSPRITE_H

#include "resources/spritedef.h"

#include <map>
#include <string>

class Animation;
class Graphics;
struct Frame;

/**
 * Animates a sprite by adding playback state.
 */
class AnimatedSprite
{
    public:
        /**
         * Constructor.
         * @param sprite the sprite to animate
         */
        AnimatedSprite(SpriteDef *sprite);

        /**
         * An helper function, which will request the sprite to animate
         * from the resource manager.
         *
         * @param filename the file of the sprite to animate
         * @param variant  the sprite variant
         */
        static AnimatedSprite *load(const std::string &filename,
                                    int variant = 0);

        /**
         * Destructor.
         */
        ~AnimatedSprite();

        /**
         * Resets the animated sprite.
         */
        void reset();

        /**
         * Plays an action using the current direction
         */
        void play(SpriteAction action);

        /**
         * Inform the animation of the passed time so that it can output the
         * correct animation frame.
         */
        void update(int time);

        /**
         * Draw the current animation frame at the coordinates given in screen
         * pixels.
         */
        bool draw(Graphics* graphics, int posX, int posY) const;

        /**
         * gets the width in pixels of the image of the current frame
         */
        int getWidth() const;

        /**
         * gets the height in pixels of the image of the current frame
         */
        int getHeight() const;

        /**
         * Sets the direction.
         */
        void setDirection(SpriteDirection direction);

    private:
        bool updateCurrentAnimation(unsigned int dt);

        SpriteDirection mDirection;    /**< The sprite direction. */
        int mLastTime;                 /**< The last time update was called. */

        int mFrameIndex;               /**< The index of the current frame. */
        int mFrameTime;                /**< The time since start of frame. */

        SpriteDef *mSprite;            /**< The sprite definition. */
        Action *mAction;               /**< The currently active action. */
        Animation *mAnimation;         /**< The currently active animation. */
        Frame *mFrame;                 /**< The currently active frame. */
};

#endif
