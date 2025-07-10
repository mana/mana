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

#include "gui/truetypefont.h"
#include "resources/theme.h"

#include <guichan/exception.hpp>


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

bool Graphics::drawRescaledImage(const Image *image, int x, int y, int width, int height)
{
    if (!image)
        return false;

    return drawRescaledImage(image, 0, 0, x, y, image->getWidth(), image->getHeight(), width, height);
}

bool Graphics::drawRescaledImageF(const Image *image,
                                  int srcX, int srcY,
                                  float dstX, float dstY,
                                  int width, int height,
                                  float desiredWidth, float desiredHeight,
                                  bool useColor)
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

void Graphics::drawRescaledImagePattern(const Image *image,
                                        int x, int y,
                                        int w, int h,
                                        int scaledWidth, int scaledHeight)
{
    drawRescaledImagePattern(image,
                             0, 0,
                             image->getWidth(),
                             image->getHeight(),
                             x, y,
                             w, h,
                             scaledWidth,
                             scaledHeight);
}

void Graphics::drawImageRect(const ImageRect &imgRect, int x, int y, int w, int h)
{
    const int srcGridX[4] = {0,
                             imgRect.left,
                             imgRect.image->getWidth() - imgRect.right,
                             imgRect.image->getWidth()};
    const int srcGridY[4] = {0,
                             imgRect.top,
                             imgRect.image->getHeight() - imgRect.bottom,
                             imgRect.image->getHeight()};

    const int dstGridX[4] = {x, x + imgRect.left, x + w - imgRect.right, x + w};
    const int dstGridY[4] = {y, y + imgRect.top, y + h - imgRect.bottom, y + h};

    for (unsigned ix = 0; ix < 3; ix++)
    {
        for (unsigned iy = 0; iy < 3; iy++)
        {
            const int srcW = srcGridX[ix + 1] - srcGridX[ix];
            const int srcH = srcGridY[iy + 1] - srcGridY[iy];

            const int dstW = dstGridX[ix + 1] - dstGridX[ix];
            const int dstH = dstGridY[iy + 1] - dstGridY[iy];

            if (srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0)
                continue;

            switch (imgRect.fillMode)
            {
            case FillMode::Stretch:
                drawRescaledImage(imgRect.image.get(),
                                  srcGridX[ix],
                                  srcGridY[iy],
                                  dstGridX[ix],
                                  dstGridY[iy],
                                  srcW, srcH,
                                  dstW, dstH);
                break;
            case FillMode::Repeat:
                drawRescaledImagePattern(imgRect.image.get(),
                                         srcGridX[ix],
                                         srcGridY[iy],
                                         srcW, srcH,
                                         dstGridX[ix],
                                         dstGridY[iy],
                                         dstW, dstH,
                                         srcW, srcH);
                break;
            }
        }
    }
}

void Graphics::drawText(const std::string &text,
                        int x, int y,
                        gcn::Graphics::Alignment alignment,
                        const gcn::Color &color,
                        gcn::Font *font,
                        bool outline,
                        bool shadow,
                        const std::optional<gcn::Color> &outlineColor,
                        const std::optional<gcn::Color> &shadowColor)
{
    switch (alignment)
    {
    case gcn::Graphics::LEFT:
        break;
    case gcn::Graphics::CENTER:
        x -= font->getWidth(text) / 2;
        break;
    case gcn::Graphics::RIGHT:
        x -= font->getWidth(text);
        break;
    default:
        throw GCN_EXCEPTION("Unknown alignment.");
    }

    auto realOutlineColor = outlineColor;
    auto realShadowColor = shadowColor;

    if (shadow && !realShadowColor)
    {
        auto sc = Theme::getThemeColor(Theme::SHADOW);
        sc.a = color.a / 2;
        realShadowColor = sc;
    }

    if (outline && !realOutlineColor)
    {
        auto oc = Theme::getThemeColor(Theme::OUTLINE);
        oc.a = color.a;
        realOutlineColor = oc;
    }

    setColor(color);
    static_cast<TrueTypeFont*>(font)->drawString(graphics, text, x, y,
                                                 realOutlineColor,
                                                 realShadowColor);
}

void Graphics::drawText(const std::string &text,
                        int x,
                        int y,
                        gcn::Graphics::Alignment align,
                        gcn::Font *font,
                        const TextFormat &format)
{
    drawText(text,
             x,
             y,
             align,
             format.color,
             font,
             format.outlineColor.has_value(),
             format.shadowColor.has_value(),
             format.outlineColor,
             format.shadowColor);
}

void Graphics::_beginDraw()
{
    pushClipArea(gcn::Rectangle(0, 0, mWidth, mHeight));
}

void Graphics::_endDraw()
{
    popClipArea();
}

void Graphics::pushClipRect(const gcn::Rectangle &rect)
{
    const gcn::ClipRectangle &carea = mClipStack.top();
    mClipRects.emplace(rect.x + carea.xOffset,
                       rect.y + carea.yOffset,
                       rect.width,
                       rect.height);

    updateClipRect();
}

void Graphics::popClipRect()
{
    mClipRects.pop();
    updateClipRect();
}
