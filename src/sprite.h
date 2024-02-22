/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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

#ifndef SPRITE_H
#define SPRITE_H

#include "resources/spritedef.h"

class Graphics;
class Image;

// Default frame display delay in milliseconds
const int DEFAULT_FRAME_DELAY = 75;

class Sprite
{
    public:
        virtual ~Sprite() = default;

        /**
         * Resets the sprite.
         *
         * @returns true if the sprite changed, false otherwise
         */
        virtual bool reset() = 0;

        /**
         * Plays an action using the current direction.
         *
         * @returns true if the sprite changed, false otherwise
         */
        virtual bool play(std::string action) = 0;

        /**
         * Inform the animation of the passed time so that it can output the
         * correct animation frame.
         *
         * @returns true if the sprite changed, false otherwise
         */
        virtual bool update(int time) = 0;

        /**
         * Draw the current animation frame at the coordinates given in screen
         * pixels.
         */
        virtual bool draw(Graphics* graphics, int posX, int posY) const = 0;

        /**
         * Gets the width in pixels of the image
         */
        virtual int getWidth() const = 0;

        /**
         * Gets the height in pixels of the image
         */
        virtual int getHeight() const = 0;

        /**
         * Gets the horizontal offset that the sprite will be drawn at
         */
        virtual int getOffsetX() const
        { return 0; }

        /**
         * Gets the vertical offset that the sprite will be drawn at
         */
        virtual int getOffsetY() const
        { return 0; }

        /**
         * Returns a reference to the current image being drawn.
         */
        virtual const Image *getImage() const = 0;

        /**
         * Sets the direction.
         *
         * @returns true if the sprite changed, false otherwise
         */
        virtual bool setDirection(SpriteDirection direction) = 0;

        /**
         * Sets the alpha value of the animated sprite
         */
        virtual void setAlpha(float alpha)
        { mAlpha = alpha; }

        /**
         * Returns the current alpha opacity of the animated sprite.
         */
        virtual float getAlpha() const
        { return mAlpha; }

        /**
         * Returns the duration of the current sprite animation in milliseconds.
         */
        virtual int getDuration() const = 0;

    protected:
        float mAlpha = 1.0f;        /**< The alpha opacity used to draw */
};

#endif // SPRITE_H
