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

#ifndef _IMAGE_H
#define _IMAGE_H

#include <allegro.h>
#include <string>
#include <vector>
#include <iostream>
#include "../log.h"

/**
 * A video image stored in memory.
 */
class VideoImage {
    private:
        BITMAP *src;
        int offset_x, offset_y;

    public:
        /**
         * Creates a VideoImage
         * @param src is a reference to a BITMAP
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
        std::vector<VideoImage*> spriteset;

        /*
         * Cuts the passed bitmap in a grid of sub bitmaps.
         */
        Spriteset::Spriteset(BITMAP *bmp, int w, int h, int offx, int offy);

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
