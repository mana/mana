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

#pragma once

#include <SDL.h>

#include <guichan/color.hpp>
#include <guichan/graphics.hpp>

class Image;

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

    ImageRect();
    ImageRect(const ImageRect &) = delete;
    ImageRect(ImageRect &&);
    ~ImageRect();

    ImageRect &operator=(const ImageRect &) = delete;
    ImageRect &operator=(ImageRect &&r) = delete;

    Image *grid[9];

    void setAlpha(float alpha);
};

/**
 * A central point of control for graphics.
 */
class Graphics : public gcn::Graphics
{
    public:
        Graphics() = default;

        /**
         * Sets whether vertical refresh syncing is enabled.
         */
        virtual void setVSync(bool sync) = 0;

        /**
         * Called when the window size or scale has changed.
         */
        virtual void updateSize(int width, int height, float scale);

        using gcn::Graphics::drawImage;

        /**
         * Blits an image onto the screen.
         *
         * @return <code>true</code> if the image was blitted properly
         *         <code>false</code> otherwise.
         */
        bool drawImage(const Image *image, int x, int y);

        /**
         * Blits an image onto the screen.
         *
         * @return <code>true</code> if the image was blitted properly
         *         <code>false</code> otherwise.
         */
        bool drawImageF(const Image *image, float x, float y);

        /**
         * Draws a rescaled version of the image.
         */
        virtual bool drawRescaledImage(const Image *image, int srcX, int srcY,
                                       int dstX, int dstY,
                                       int width, int height,
                                       int desiredWidth, int desiredHeight,
                                       bool useColor = false) = 0;

        /**
         * Draws a rescaled version of the image.
         */
        virtual bool drawRescaledImageF(const Image *image, int srcX, int srcY,
                                        float dstX, float dstY,
                                        int width, int height,
                                        float desiredWidth, float desiredHeight,
                                        bool useColor = false);

        /**
         * Blits an image onto the screen.
         *
         * @return <code>true</code> if the image was blitted properly
         *         <code>false</code> otherwise.
         */
        virtual bool drawImage(const Image *image,
                               int srcX, int srcY,
                               int dstX, int dstY,
                               int width, int height,
                               bool useColor = false);

        /**
         * Blits an image onto the screen.
         *
         * @return <code>true</code> if the image was blitted properly
         *         <code>false</code> otherwise.
         */
        virtual bool drawImageF(const Image *image,
                                int srcX, int srcY,
                                float dstX, float dstY,
                                int width, int height,
                                bool useColor = false);

        virtual void drawImagePattern(const Image *image,
                                      int x, int y,
                                      int w, int h);

        /**
         * Draw a pattern based on a rescaled version of the given image...
         */
        virtual void drawRescaledImagePattern(const Image *image,
                                              int x, int y,
                                              int w, int h,
                                              int scaledWidth,
                                              int scaledHeight) = 0;

        /**
         * Draws a rectangle using images. 4 corner images, 4 side images and 1
         * image for the inside.
         */
        void drawImageRect(int x, int y, int w, int h,
                           const Image *topLeft, const Image *topRight,
                           const Image *bottomLeft, const Image *bottomRight,
                           const Image *top, const Image *right,
                           const Image *bottom, const Image *left,
                           const Image *center);

        /**
         * Draws a rectangle using images. 4 corner images, 4 side images and 1
         * image for the inside.
         */
        void drawImageRect(int x, int y, int w, int h,
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
        virtual void updateScreen() = 0;

        /**
         * Returns the logical width of the screen.
         */
        int getWidth() const { return mWidth; }

        /**
         * Returns the logical height of the screen.
         */
        int getHeight() const { return mHeight; }

        /**
         * Returns the graphics scale.
         */
        float getScale() const { return mScale; }

        /**
         * Converts a window coordinate to a logical coordinate. Used for
         * converting mouse coordinates.
         */
        virtual void windowToLogical(int windowX, int windowY,
                                     float &logicalX, float &logicalY) const = 0;

        void _beginDraw() override;
        void _endDraw() override;

        /**
         * Takes a screenshot and returns it as SDL surface.
         */
        virtual SDL_Surface *getScreenshot() = 0;

        gcn::Font *getFont() const { return mFont; }

        void drawImage(const gcn::Image *image,
                       int srcX, int srcY,
                       int dstX, int dstY,
                       int width, int height) override {}   // not used

        void setColor(const gcn::Color &color) override
        {
            mColor = color;
        }

        const gcn::Color &getColor() const final
        {
            return mColor;
        }

    protected:
        int mWidth = 0;
        int mHeight = 0;
        float mScale = 1.0f;
        gcn::Color mColor;
};

extern Graphics *graphics;
