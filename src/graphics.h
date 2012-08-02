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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <guichan/sdl/sdlgraphics.hpp>

class Image;
class ImageRect;

struct SDL_Surface;

static const int defaultScreenWidth = 800;
static const int defaultScreenHeight = 600;

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
struct ImageRect
{
    enum ImagePosition
    {
        UPPER_LEFT = 0,
        UPPER_CENTER = 1,
        UPPER_RIGHT = 2,
        LEFT = 3,
        CENTER = 4,
        RIGHT = 5,
        LOWER_LEFT = 6,
        LOWER_CENTER = 7,
        LOWER_RIGHT = 8
    };

    Image *grid[9];
};

/**
 * A central point of control for graphics.
 */
class Graphics : public gcn::SDLGraphics
{
    public:
        enum BlitMode {
            BLIT_NORMAL = 0,
            BLIT_GFX
        };

        Graphics();

        virtual ~Graphics();

        /**
         * Try to create a window with the given settings.
         */
        virtual bool setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel);

        /**
         * Change the video mode. Can be used for switching to full screen,
         * changing resolution or adapting after window resize.
         */
        bool changeVideoMode(int w, int h, int bpp, bool fs, bool hwaccel);

        /**
         * Blits an image onto the screen.
         *
         * @return <code>true</code> if the image was blitted properly
         *         <code>false</code> otherwise.
         */
        bool drawImage(Image *image, int x, int y);

        /**
         * Draws a rescaled version of the image.
         */
        bool drawRescaledImage(Image *image, int srcX, int srcY,
                               int dstX, int dstY,
                               int width, int height,
                               int desiredWidth, int desiredHeight)
        { return drawRescaledImage(image, srcX, srcY,
                                   dstX, dstY,
                                   width, height,
                                   desiredWidth, desiredHeight,
                                   false); }

        /**
         * Draws a rescaled version of the image.
         */
        virtual bool drawRescaledImage(Image *image, int srcX, int srcY,
                               int dstX, int dstY,
                               int width, int height,
                               int desiredWidth, int desiredHeight,
                               bool useColor = false);

        /**
         * Blits an image onto the screen.
         *
         * @return <code>true</code> if the image was blitted properly
         *         <code>false</code> otherwise.
         */
        virtual bool drawImage(Image *image,
                               int srcX, int srcY,
                               int dstX, int dstY,
                               int width, int height,
                               bool useColor = false);

        virtual void drawImagePattern(Image *image,
                                      int x, int y,
                                      int w, int h);

        /**
         * Draw a pattern based on a rescaled version of the given image...
         */
        virtual void drawRescaledImagePattern(Image *image,
                               int x, int y, int w, int h,
                               int scaledWidth, int scaledHeight);

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
         * Draws a rectangle using images. 4 corner images, 4 side images and 1
         * image for the inside.
         */
        void drawImageRect(const gcn::Rectangle &area,
                           const ImageRect &imgRect)
        {
            drawImageRect(area.x, area.y, area.width, area.height, imgRect);
        }

        void setBlitMode(BlitMode mode)
        { mBlitMode = mode; }

        BlitMode getBlitMode()
        { return mBlitMode; }

        /**
         * Updates the screen. This is done by either copying the buffer to the
         * screen or swapping pages.
         */
        virtual void updateScreen();

        /**
         * Returns the width of the drawable surface.
         */
        int getWidth() const { return mWidth; }

        /**
         * Returns the height of the drawable surface.
         */
        int getHeight() const { return mHeight; }

        /**
         * Returns the current scale ratio of the screen.
         */
        int getScale() const { return mScale; }

        /**
         * Returns the amount of bits per pixel that was requested (not the
         * actual amount that's currently active).
         */
        int getBpp() const { return mBpp; }

        /**
         * Returns whether we're in a full screen mode.
         */
        bool getFullscreen() const { return mFullscreen; }

        /**
         * Returns whether old-fashioned SDL-based hardware acceleration was
         * requested (not whether it's currently active).
         */
        bool getHWAccel() const { return mHWAccel; }

        /**
         * Takes a screenshot and returns it as SDL surface.
         */
        virtual SDL_Surface *getScreenshot();

        gcn::Font *getFont() const { return mFont; }

    protected:
        int mWidth;
        int mHeight;
        int mScale;
        int mBpp;
        bool mFullscreen;
        bool mHWAccel;
        BlitMode mBlitMode;
        SDL_Surface *mScreenSurface;
};

extern Graphics *graphics;

#endif
