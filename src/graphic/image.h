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
 *  By ElvenProgrammer aka Eugenio Favalli (umperio@users.sourceforge.net)
 */

#ifndef _IMAGE_H
#define _IMAGE_H

#include <allegro.h>
#include <string>
#include <vector>
#include <iostream>
#include "../log.h"

/**
 * A class storing a single sprite in video memory if available,
 * else as a refernece to a RLE_SPRITE in a datafile.
 * The class stores the offsets needed to compensate the cropping
 * operated by the grabber utility.
 */
class Image {
    public:
        /**
         * Creates an Image
         * @param offset_x is the x offset from where to start drawing
         * @param offset_y is the y offset from where to start drawing
         */
        Image(int offset_x, int offset_y);

        /**
         * Virtual function to draw a sprite
         * @param dest is the destination bitmap on which to draw the sprite
         * @param x is the horizontal position
         * @param y is the vertical position
         */ 
        virtual void draw(BITMAP *dest, int x, int y) = 0;

    protected:
        // From where to start drawing
        int offset_x, offset_y;
};

/**
 * A RLE sprite
 */
class RleImage : public Image {
    public:
        /**
         * Creates a RleSprite
         * @param src is a reference to a RLE_SPRITE in a datafile
         * @param offset_x is the x offset from where to start drawing
         * @param offset_y is the y offset from where to start drawing
         */
        RleImage(RLE_SPRITE *src, int offset_x, int offset_y);

        /**
         * Destructor
         */
        virtual ~RleImage();

        /**
         * Draws a sprite
         */
        void draw(BITMAP *dest, int x, int y);

    private:
        // Reference to RLE_SPRITE
        RLE_SPRITE *src;
};

/**
 * An image stored in video memory
 */
class VideoImage : public Image {
    private:
        // Reference to bitmap stored in video memory
        BITMAP *src;
    public:
        /**
         * Creates a VideoImage
         * @param src is a reference to a BITMAP in video memory
         * @param offset_x is the x offset from where to start drawing
         * @param offset_y is the y offset from where to start drawing
         */        
        VideoImage(BITMAP *src, int offset_x, int offset_y);

        /**
         * Destructor
         */
        virtual ~VideoImage();

        /**
         * Draws a sprite
         */
        void draw(BITMAP *dest, int x, int y);
};

/**
 * Stores a complete set of sprites.
 */
class Spriteset {
    public:
        // Vector storing the whole spriteset.
        std::vector<Image *> spriteset;

        /**
         * Load a datafile containing the spriteset
         * @param filename is the path of the datafile
         */
        Spriteset(std::string filename);

        /*
         * Cuts the passed bitmap in a grid of sub bitmaps.
         */
        Spriteset::Spriteset(BITMAP *bmp, int width, int height);

        /**
         * Destructor
         */
        ~Spriteset();

    private:
        /**
         * Helper function to get offset
         * @param datafile is a reference to the whole spriteset
         * @param type is the property of the datafile object         
         */
        int getProperty(DATAFILE *datafile, int type);
};

#endif
