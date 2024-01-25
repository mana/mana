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

#include <SDL.h>

#include <guichan/color.hpp>
#include <guichan/graphics.hpp>

class Image;
class ImageRect;

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
class ImageRect
{
public:
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
class Graphics : public gcn::Graphics
{
    public:
        Graphics() = default;

        virtual ~Graphics();

        /**
         * Sets the target SDL_Window to draw to. This funtion also pushes a
         * clip areas corresponding to the dimension of the target.
         *
         * @param target the target to draw to.
         */
        virtual void setTarget(SDL_Window *target);

        SDL_Window *getTarget() const { return mTarget; }
        SDL_Renderer *getRenderer() const { return mRenderer; }

        /**
         * Try to create a window with the given settings.
         */
        virtual bool setVideoMode(int w, int h, bool fs);

        /**
         * Change the video mode. Can be used for switching to full screen,
         * changing resolution or adapting after window resize.
         */
        bool changeVideoMode(int w, int h, bool fs);

        virtual void videoResized(int w, int h);

        /**
         * Blits an image onto the screen.
         *
         * @return <code>true</code> if the image was blitted properly
         *         <code>false</code> otherwise.
         */
        bool drawImage(Image *image, int x, int y);

        /**
         * Draws a resclaled version of the image
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
         * Draws a resclaled version of the image
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

        /**
         * Updates the screen. This is done by either copying the buffer to the
         * screen or swapping pages.
         */
        virtual void updateScreen();

        /**
         * Returns the width of the screen.
         */
        int getWidth() const;

        /**
         * Returns the height of the screen.
         */
        int getHeight() const;

        /**
         * Returns whether we're in a full screen mode.
         */
        bool getFullscreen() const { return mFullscreen; }

        /**
         * Takes a screenshot and returns it as SDL surface.
         */
        virtual SDL_Surface *getScreenshot();

        gcn::Font *getFont() const { return mFont; }

        virtual bool pushClipArea(gcn::Rectangle area);

        virtual void popClipArea();

        virtual void drawImage(const gcn::Image *image,
                               int srcX,
                               int srcY,
                               int dstX,
                               int dstY,
                               int width,
                               int height) {}   // not used

        virtual void drawPoint(int x, int y);

        virtual void drawLine(int x1, int y1, int x2, int y2);

        virtual void drawRectangle(const gcn::Rectangle &rectangle);

        virtual void fillRectangle(const gcn::Rectangle &rectangle);

        virtual void setColor(const gcn::Color &color)
        {
            mColor = color;
        }

        virtual const gcn::Color &getColor() const
        {
            return mColor;
        }

    protected:
        void updateSDLClipRect();

        int mWidth = 0;
        int mHeight = 0;
        bool mFullscreen = false;
        gcn::Color mColor;

        SDL_Window *mTarget = nullptr;
        SDL_Renderer *mRenderer = nullptr;
};

extern Graphics *graphics;

#endif
