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

#include <guichan/sdl.hpp>
#include <SDL.h>
#ifdef USE_OPENGL
#include <guichan/opengl.hpp>
#include <SDL_opengl.h>
#endif
#include <guichan/imagefont.hpp>
#include <guichan/rectangle.hpp>
#include "resources/image.h"

/**
 * 9 images defining a rectangle. 4 corners, 4 sides and a middle area. The
 * topology is as follows:
 *
 * <pre>
 *  !-----!-----------------!-----!
 *  !  0  !        1        !  2  !
 *  !-----!-----------------!-----!
 *  !  3  !        4        !  5  !
 *  !-----!-----------------!-----!
 *  !  6  !        7        !  8  !
 *  !-----!-----------------!-----!
 * </pre>
 *
 * Sections 0, 2, 6 and 8 will remain as is. 1, 3, 4, 5 and 7 will be
 * repeated to fit the size of the widget.
 */
struct ImageRect {
    Image *grid[9];
};

/**
 * A central point of control for graphics.
 */
class Graphics :
#ifdef USE_OPENGL
public gcn::OpenGLGraphics,
#endif
public gcn::SDLGraphics {
    public:
        /**
         * Constructor.
         */
        Graphics();

        /**
         * Destructor.
         */
        ~Graphics();

        /**
         * Try to create a window with the given settings.
         */
        bool setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel);

        /**
         * Set fullscreen mode.
         */
        bool setFullscreen(bool fs);

        void _beginDraw();
        void _endDraw();

        void drawImage(Image *image, int x, int y);
        void drawImagePattern(Image *image, int x, int y, int w, int h);

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
        void updateScreen();

        /**
         * Returns the width of the screen.
         */
        int getWidth();

        /**
         * Returns the height of the screen.
         */
        int getHeight();

        void setFont(gcn::ImageFont *font);

        void drawText(const std::string &text,
                int x,
                int y,
                unsigned int alignment);

        void setColor(gcn::Color color);

    private:
        SDL_Surface *mScreen;
        bool mFullscreen, mHWAccel;
};

#endif
