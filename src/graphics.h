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

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include <guichan/sdl/sdlgraphics.hpp>

#include "guichanfwd.h"

class Image;
class ImageRect;

class SDL_Surface;

/**
 * A central point of control for graphics.
 */
class Graphics : public gcn::SDLGraphics {
    public:
        /**
         * Constructor.
         */
        Graphics();

        /**
         * Destructor.
         */
        virtual ~Graphics();

        /**
         * Try to create a window with the given settings.
         */
        virtual bool setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel);

        /**
         * Set fullscreen mode.
         */
        bool setFullscreen(bool fs);

        /**
         * Blits an image onto the screen.
         *
         * @return <code>true</code> if the image was blitted properly
         *         <code>false</code> otherwise.
         */
        bool drawImage(Image *image, int x, int y);

        /**
         * Blits an image onto the screen.
         *
         * @return <code>true</code> if the image was blitted properly
         *         <code>false</code> otherwise.
         */
        virtual bool drawImage(Image *image, int srcX, int srcY, int dstX, int dstY, int width, int height);

        virtual void drawImagePattern(Image *image, int x, int y, int w, int h);

        /**
         * Draws a rectangle using images. 4 corner images, 4 side images and 1
         * image for the inside.
         */
        void drawImageRect(
                int x, int y, int w, int h,
                Image *topLeft, Image *topRight,
                Image *bottomLeft, Image *bottomRight,
                Image *top, Image *right,
                Image *bottom, Image *left,
                Image *center);

        /**
         * Draws a rectangle using images. 4 corner images, 4 side images and 1
         * image for the inside.
         */
        void drawImageRect(
                int x, int y, int w, int h,
                const ImageRect &imgRect);

        /**
         * Updates the screen. This is done by either copying the buffer to the
         * screen or swapping pages.
         */
        virtual void updateScreen();

        /**
         * Returns the width of the screen.
         */
        int getWidth();

        /**
         * Returns the height of the screen.
         */
        int getHeight();

    protected:
        SDL_Surface *mScreen;
        bool mFullscreen, mHWAccel;
};

#endif
