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

#ifndef _TMW_SPRITE_H_
#define _TMW_SPRITE_H_

#include "graphics.h"

/**
 * A sprite is some visible object on a map.
 */
class Sprite
{
    public:
        /**
         * Constructor.
         */
        Sprite(int x = 0, int y = 0, Image *image = NULL):
            mX(x),
            mY(y),
            mImage(image)
        {
        }

        /**
         * Draws the sprite to the given graphics context.
         *
         * Note: this function could be simplified if the graphics context
         * would support setting a translation offset. It already does this
         * partly with the clipping rectangle support.
         */
        void
        draw(Graphics *graphics, int offsetX, int offsetY)
        {
            graphics->drawImage(mImage, mX + offsetX, mY + offsetY);
        }

        /**
         * Returns the X coordinate of the sprite.
         */
        int
        getY() const { return mY; }

        /**
         * Returns the Y coordinate of the sprite.
         */
        int
        getX() const { return mX; }

    private:
        int mX;           /**< X coordinate in pixels. */
        int mY;           /**< Y coordinate in pixels. */
        Image *mImage;    /**< The image currently representing this sprite. */
};

#endif
