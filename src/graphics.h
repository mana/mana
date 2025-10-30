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

#include "resources/image.h"

#include <SDL.h>

#include <guichan/color.hpp>
#include <guichan/graphics.hpp>

#include <memory>
#include <optional>

struct TextFormat;

enum class FillMode
{
    Stretch,
    Repeat,
};

/**
 * An image reference along with the margins specifying how to render this
 * image at different sizes. The margins divide the image into 9 sections as
 * follows:
 *
 * <pre>
 *  !------!--------------!-------!
 *  !      !      top     !       !
 *  !------!--------------!-------!
 *  ! left !              ! right !
 *  !------!--------------!-------!
 *  !      !     bottom   !       !
 *  !------!--------------!-------!
 * </pre>
 *
 * The corner sections will remain as is. The edges and the center sections
 * will be repeated or stretched to fit the target size, depending on the fill
 * mode.
 */
struct ImageRect
{
    std::unique_ptr<Image> image;
    int top = 0;
    int left = 0;
    int bottom = 0;
    int right = 0;
    FillMode fillMode = FillMode::Stretch;

    int minWidth() const { return left + right; }
    int minHeight() const { return top + bottom; }
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
        bool drawRescaledImage(const Image *image, int x, int y, int width, int height);

        /**
         * Draws a rescaled version of the image.
         */
        virtual bool drawRescaledImage(const Image *image,
                                       int srcX, int srcY,
                                       int dstX, int dstY,
                                       int width, int height,
                                       int desiredWidth, int desiredHeight) = 0;

        /**
         * Draws a rescaled version of the image.
         */
        virtual bool drawRescaledImageF(const Image *image, int srcX, int srcY,
                                        float dstX, float dstY,
                                        int width, int height,
                                        float desiredWidth, float desiredHeight);

        /**
         * Blits an image onto the screen.
         *
         * @return <code>true</code> if the image was blitted properly
         *         <code>false</code> otherwise.
         */
        virtual bool drawImage(const Image *image,
                               int srcX, int srcY,
                               int dstX, int dstY,
                               int width, int height);

        /**
         * Blits an image onto the screen.
         *
         * @return <code>true</code> if the image was blitted properly
         *         <code>false</code> otherwise.
         */
        virtual bool drawImageF(const Image *image,
                                int srcX, int srcY,
                                float dstX, float dstY,
                                int width, int height);

        virtual void drawImagePattern(const Image *image,
                                      int x, int y,
                                      int w, int h);

        /**
         * Draw a pattern based on a rescaled version of the given image.
         */
        void drawRescaledImagePattern(const Image *image,
                                      int x, int y,
                                      int w, int h,
                                      int scaledWidth,
                                      int scaledHeight);

        /**
         * Draw a pattern based on a rescaled version of the given image.
         */
        virtual void drawRescaledImagePattern(const Image *image,
                                              int srcX, int srcY,
                                              int srcW, int srcH,
                                              int dstX, int dstY,
                                              int dstW, int dstH,
                                              int scaledWidth,
                                              int scaledHeight) = 0;

        /**
         * Draws a rectangle using images. 4 corner images, 4 side images and 1
         * image for the inside.
         */
        void drawImageRect(const ImageRect &imgRect, int x, int y, int w, int h);

        void drawImageRect(const ImageRect &imgRect, const gcn::Rectangle &area)
        {
            drawImageRect(imgRect, area.x, area.y, area.width, area.height);
        }

        using gcn::Graphics::drawText;

        void drawText(const std::string &text,
                      int x, int y,
                      gcn::Graphics::Alignment alignment,
                      const gcn::Color &color,
                      gcn::Font *font,
                      bool outline = false,
                      bool shadow = false,
                      const std::optional<gcn::Color> &outlineColor = {},
                      const std::optional<gcn::Color> &shadowColor = {});

        void drawText(const std::string &text,
                      int x,
                      int y,
                      gcn::Graphics::Alignment align,
                      gcn::Font *font,
                      const TextFormat &format);

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

        void pushClipRect(const gcn::Rectangle &rect);
        void popClipRect();

    protected:
        virtual void updateClipRect() = 0;

        int mWidth = 0;
        int mHeight = 0;
        float mScale = 1.0f;
        gcn::Color mColor;

        // Actual clipping rects. Clipping by gcn::Graphics::mClipStack is disabled.
        std::stack<gcn::Rectangle> mClipRects;
};

extern Graphics *graphics;
