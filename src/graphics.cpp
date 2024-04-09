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

#include "graphics.h"

#include "resources/image.h"

#include <guichan/exception.hpp>

void ImageRect::setAlpha(float alpha)
{
    for (auto img : grid)
    {
        img->setAlpha(alpha);
    }
}

void Graphics::updateSize(int width, int height, float /*scale*/)
{
    mWidth = width;
    mHeight = height;
}

bool Graphics::drawImage(const Image *image, int x, int y)
{
    if (!image)
        return false;

    return drawImage(image, 0, 0, x, y, image->getWidth(), image->getHeight());
}

bool Graphics::drawImageF(const Image *image, float x, float y)
{
    if (!image)
        return false;

    return drawImageF(image, 0, 0, x, y, image->getWidth(), image->getHeight());
}

bool Graphics::drawRescaledImageF(const Image *image, int srcX, int srcY, float dstX, float dstY, int width, int height, float desiredWidth, float desiredHeight, bool useColor)
{
    return drawRescaledImage(image,
                             srcX, srcY,
                             static_cast<int>(dstX),
                             static_cast<int>(dstY),
                             width, height,
                             static_cast<int>(desiredWidth),
                             static_cast<int>(desiredHeight),
                             useColor);
}

bool Graphics::drawImage(const Image *image,
                         int srcX, int srcY,
                         int dstX, int dstY,
                         int width, int height,
                         bool useColor)
{
    return drawRescaledImage(image,
                             srcX, srcY,
                             dstX, dstY,
                             width, height,
                             width, height, useColor);
}

bool Graphics::drawImageF(const Image *image, int srcX, int srcY, float dstX, float dstY, int width, int height, bool useColor)
{
    return drawRescaledImageF(image,
                              srcX, srcY,
                              dstX, dstY,
                              width, height,
                              width, height, useColor);
}

void Graphics::drawImagePattern(const Image *image, int x, int y, int w, int h)
{
    if (!image)
        return;

    drawRescaledImagePattern(image, x, y, w, h,
                             image->getWidth(), image->getHeight());
}

void Graphics::drawImageRect(int x, int y, int w, int h,
                             const Image *topLeft, const Image *topRight,
                             const Image *bottomLeft, const Image *bottomRight,
                             const Image *top, const Image *right,
                             const Image *bottom, const Image *left,
                             const Image *center)
{
    pushClipArea(gcn::Rectangle(x, y, w, h));

    // Draw the center area
    drawImagePattern(center,
            topLeft->getWidth(), topLeft->getHeight(),
            w - topLeft->getWidth() - topRight->getWidth(),
            h - topLeft->getHeight() - bottomLeft->getHeight());

    // Draw the sides
    drawImagePattern(top,
            left->getWidth(), 0,
            w - left->getWidth() - right->getWidth(), top->getHeight());
    drawImagePattern(bottom,
            left->getWidth(), h - bottom->getHeight(),
            w - left->getWidth() - right->getWidth(),
            bottom->getHeight());
    drawImagePattern(left,
            0, top->getHeight(),
            left->getWidth(),
            h - top->getHeight() - bottom->getHeight());
    drawImagePattern(right,
            w - right->getWidth(), top->getHeight(),
            right->getWidth(),
            h - top->getHeight() - bottom->getHeight());

    // Draw the corners
    drawImage(topLeft, 0, 0);
    drawImage(topRight, w - topRight->getWidth(), 0);
    drawImage(bottomLeft, 0, h - bottomLeft->getHeight());
    drawImage(bottomRight,
            w - bottomRight->getWidth(),
            h - bottomRight->getHeight());

    popClipArea();
}

void Graphics::drawImageRect(int x, int y, int w, int h,
                             const ImageRect &imgRect)
{
    drawImageRect(x, y, w, h,
            imgRect.grid[0], imgRect.grid[2], imgRect.grid[6], imgRect.grid[8],
            imgRect.grid[1], imgRect.grid[5], imgRect.grid[7], imgRect.grid[3],
            imgRect.grid[4]);
}

void Graphics::_beginDraw()
{
    pushClipArea(gcn::Rectangle(0, 0, mWidth, mHeight));
}

void Graphics::_endDraw()
{
    popClipArea();
}
