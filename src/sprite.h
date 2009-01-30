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

#ifndef SPRITE_H
#define SPRITE_H

class Graphics;

/**
 * A sprite is some visible object on a map. This abstract class defines the
 * interface used by the map to sort and display the sprite.
 */
class Sprite
{
    public:
        /**
         * Destructor.
         */
        virtual
        ~Sprite() {}

        /**
         * Draws the sprite to the given graphics context.
         *
         * Note: this function could be simplified if the graphics context
         * would support setting a translation offset. It already does this
         * partly with the clipping rectangle support.
         */
        virtual void draw(Graphics *graphics, int offsetX, int offsetY) const = 0;

        /**
         * Returns the horizontal size of the sprites graphical representation
         * in pixels or 0 when it is undefined.
         */
        virtual int getWidth() const
        { return 0; }

        /**
         * Returns the vertical size of the sprites graphical representation
         * in pixels or 0 when it is undefined.
         */
        virtual int getHeight() const
        { return 0; }

        /**
         * Returns the pixel Y coordinate of the sprite.
         */
        virtual int getPixelY() const = 0;
};

#endif
