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
#include "log.h"

#include "resources/image.h"

#include "utils/gettext.h"

#include <guichan/exception.hpp>

void ImageRect::setAlpha(float alpha)
{
    for (auto img : grid)
    {
        img->setAlpha(alpha);
    }
}


Graphics::~Graphics()
{
    _endDraw();
}

void Graphics::setTarget(SDL_Window *target)
{
    _endDraw();

    mTarget = target;

    if (mTarget)
        _beginDraw();
}

bool Graphics::setVideoMode(int w, int h, bool fs)
{
    logger->log("Setting video mode %dx%d %s",
            w, h, fs ? "fullscreen" : "windowed");

    int windowFlags = SDL_WINDOW_ALLOW_HIGHDPI;

    if (fs)
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    else
        windowFlags |= SDL_WINDOW_RESIZABLE;

    // TODO_SDL2: Support SDL_WINDOW_FULLSCREEN_DESKTOP

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_CreateWindowAndRenderer(w, h, windowFlags, &window, &renderer);

    if (!window)
        return false;

    SDL_SetWindowMinimumSize(window, 640, 480);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    setTarget(window);

    mRenderer = renderer;
    mWidth = w;
    mHeight = h;
    mFullscreen = fs;

    if (const char *driver = SDL_GetCurrentVideoDriver())
        logger->log("Using video driver: %s", driver);
    else
        logger->log("Using video driver: not initialized");

    SDL_RendererInfo info;

    if (SDL_GetRendererInfo(mRenderer, &info) == 0) {
        logger->log("Using renderer: %s", info.name);

        logger->log("The renderer is a software fallback: %s",
                    (info.flags & SDL_RENDERER_SOFTWARE) ? "yes" : "no");
        logger->log("The renderer is hardware accelerated: %s",
                    (info.flags & SDL_RENDERER_ACCELERATED) ? "yes" : "no");
        logger->log("Vsync: %s",
                    (info.flags & SDL_RENDERER_PRESENTVSYNC) ? "on" : "off");
        logger->log("Renderer supports rendering to texture: %s",
                    (info.flags & SDL_RENDERER_TARGETTEXTURE) ? "yes" : "no");
        logger->log("Max texture size: %dx%d",
                    info.max_texture_width, info.max_texture_height);
    }

    return true;
}

bool Graphics::changeVideoMode(int w, int h, bool fs)
{
    // Just return success if we're already in this mode
    if (mWidth == w &&
            mHeight == h &&
            mFullscreen == fs)
        return true;

    _endDraw();

    bool success = setVideoMode(w, h, fs);

    // If it didn't work, try to restore the previous mode. If that doesn't
    // work either, we're in big trouble and bail out.
    if (!success) {
        if (!setVideoMode(mWidth, mHeight, mFullscreen)) {
            logger->error(_("Failed to change video mode and couldn't "
                            "switch back to the previous mode!"));
        }
    }

    _beginDraw();

    return success;
}

void Graphics::videoResized(int w, int h)
{
    mWidth = w;
    mHeight = h;
}

int Graphics::getWidth() const
{
    return mWidth;
}

int Graphics::getHeight() const
{
    return mHeight;
}

bool Graphics::drawImage(Image *image, int x, int y)
{
    if (!image)
        return false;

    return drawImage(image, 0, 0, x, y, image->mBounds.w, image->mBounds.h);
}

bool Graphics::drawRescaledImage(Image *image, int srcX, int srcY,
                               int dstX, int dstY,
                               int width, int height,
                               int desiredWidth, int desiredHeight,
                               bool useColor)
{
    // Check that preconditions for blitting are met.
    if (!mTarget || !image || !image->mTexture)
        return false;

    dstX += mClipStack.top().xOffset;
    dstY += mClipStack.top().yOffset;

    srcX += image->mBounds.x;
    srcY += image->mBounds.y;

    SDL_Rect dstRect;
    SDL_Rect srcRect;
    dstRect.x = dstX; dstRect.y = dstY;
    srcRect.x = srcX; srcRect.y = srcY;
    srcRect.w = width;
    srcRect.h = height;
    dstRect.w = desiredWidth;
    dstRect.h = desiredHeight;

    return !(SDL_RenderCopy(mRenderer, image->mTexture, &srcRect, &dstRect) < 0);
}

bool Graphics::drawImage(Image *image, int srcX, int srcY, int dstX, int dstY,
                         int width, int height, bool useColor)
{
    if (!image)
        return false;

    return drawRescaledImage(image,
                             srcX, srcY,
                             dstX, dstY,
                             width, height,
                             width, height, useColor);
}

void Graphics::drawImagePattern(Image *image, int x, int y, int w, int h)
{
    // Check that preconditions for blitting are met.
    if (!image)
        return;

    const int iw = image->getWidth();
    const int ih = image->getHeight();

    drawRescaledImagePattern(image, x, y, w, h, iw, ih);
}

void Graphics::drawRescaledImagePattern(Image *image,
                                        int x, int y,
                                        int w, int h,
                                        int scaledWidth, int scaledHeight)
{
    // Check that preconditions for blitting are met.
    if (!mTarget || !image || !image->mTexture)
        return;

    if (scaledHeight == 0 || scaledWidth == 0)
        return;

    for (int py = 0; py < h; py += scaledHeight)     // Y position on pattern plane
    {
        int dh = (py + scaledHeight >= h) ? h - py : scaledHeight;
        int srcY = image->mBounds.y;
        int dstY = y + py + mClipStack.top().yOffset;

        for (int px = 0; px < w; px += scaledWidth) // X position on pattern plane
        {
            int dw = (px + scaledWidth >= w) ? w - px : scaledWidth;
            int srcX = image->mBounds.x;
            int dstX = x + px + mClipStack.top().xOffset;

            SDL_Rect dstRect;
            SDL_Rect srcRect;
            dstRect.x = dstX; dstRect.y = dstY;
            dstRect.w = dw;   dstRect.h = dh;
            srcRect.x = srcX; srcRect.y = srcY;
            srcRect.w = dw;   srcRect.h = dh;

            SDL_RenderCopy(mRenderer, image->mTexture, &srcRect, &dstRect);
        }
    }
}

void Graphics::drawImageRect(int x, int y, int w, int h,
                             Image *topLeft, Image *topRight,
                             Image *bottomLeft, Image *bottomRight,
                             Image *top, Image *right,
                             Image *bottom, Image *left,
                             Image *center)
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

void Graphics::updateScreen()
{
    SDL_RenderPresent(mRenderer);
}

SDL_Surface *Graphics::getScreenshot()
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int rmask = 0xff000000;
    int gmask = 0x00ff0000;
    int bmask = 0x0000ff00;
#else
    int rmask = 0x000000ff;
    int gmask = 0x0000ff00;
    int bmask = 0x00ff0000;
#endif
    int amask = 0x00000000;

    SDL_Surface *screenshot = SDL_CreateRGBSurface(0, mWidth,
            mHeight, 24, rmask, gmask, bmask, amask);
    SDL_RenderReadPixels(mRenderer, nullptr, SDL_PIXELFORMAT_RGB888, screenshot->pixels, screenshot->pitch);

    return screenshot;
}

bool Graphics::pushClipArea(gcn::Rectangle area)
{
    bool result = gcn::Graphics::pushClipArea(area);
    updateSDLClipRect();
    return result;
}

void Graphics::popClipArea()
{
    gcn::Graphics::popClipArea();
    updateSDLClipRect();
}

void Graphics::updateSDLClipRect()
{
    if (mClipStack.empty())
    {
        SDL_RenderSetClipRect(mRenderer, nullptr);
        return;
    }

    const gcn::ClipRectangle &carea = mClipStack.top();
    SDL_Rect rect;
    rect.x = carea.x;
    rect.y = carea.y;
    rect.w = carea.width;
    rect.h = carea.height;

    SDL_RenderSetClipRect(mRenderer, &rect);
}

void Graphics::drawPoint(int x, int y)
{
    if (mClipStack.empty())
    {
        throw GCN_EXCEPTION("Clip stack is empty, perhaps you called a draw funtion outside of _beginDraw() and _endDraw()?");
    }

    const gcn::ClipRectangle &top = mClipStack.top();

    x += top.xOffset;
    y += top.yOffset;

    if (!top.isPointInRect(x, y))
        return;

    SDL_SetRenderDrawColor(mRenderer,
                           (Uint8)(mColor.r),
                           (Uint8)(mColor.g),
                           (Uint8)(mColor.b),
                           (Uint8)(mColor.a));
    SDL_RenderDrawPoint(mRenderer, x, y);
}

void Graphics::drawLine(int x1, int y1, int x2, int y2)
{
    if (mClipStack.empty())
    {
        throw GCN_EXCEPTION("Clip stack is empty, perhaps you called a draw funtion outside of _beginDraw() and _endDraw()?");
    }

    const gcn::ClipRectangle &top = mClipStack.top();

    x1 += top.xOffset;
    y1 += top.yOffset;
    x2 += top.xOffset;
    y2 += top.yOffset;

    SDL_SetRenderDrawColor(mRenderer,
                           (Uint8)(mColor.r),
                           (Uint8)(mColor.g),
                           (Uint8)(mColor.b),
                           (Uint8)(mColor.a));
    SDL_RenderDrawLine(mRenderer, x1, y1, x2, y2);
}

void Graphics::drawRectangle(const gcn::Rectangle &rectangle)
{
    if (mClipStack.empty())
    {
        throw GCN_EXCEPTION("Clip stack is empty, perhaps you called a draw funtion outside of _beginDraw() and _endDraw()?");
    }

    const gcn::ClipRectangle &top = mClipStack.top();

    SDL_Rect rect;
    rect.x = rectangle.x + top.xOffset;
    rect.y = rectangle.y + top.yOffset;
    rect.w = rectangle.width;
    rect.h = rectangle.height;

    SDL_SetRenderDrawColor(mRenderer,
                           (Uint8)(mColor.r),
                           (Uint8)(mColor.g),
                           (Uint8)(mColor.b),
                           (Uint8)(mColor.a));
    SDL_RenderDrawRect(mRenderer, &rect);
}

void Graphics::fillRectangle(const gcn::Rectangle &rectangle)
{
    if (mClipStack.empty())
    {
        throw GCN_EXCEPTION("Clip stack is empty, perhaps you called a draw funtion outside of _beginDraw() and _endDraw()?");
    }

    const gcn::ClipRectangle &top = mClipStack.top();

    gcn::Rectangle area = rectangle;
    area.x += top.xOffset;
    area.y += top.yOffset;

    if(!area.isIntersecting(top))
    {
        return;
    }

    SDL_Rect rect;
    rect.x = area.x;
    rect.y = area.y;
    rect.w = area.width;
    rect.h = area.height;

    SDL_SetRenderDrawColor(mRenderer,
                           (Uint8)(mColor.r),
                           (Uint8)(mColor.g),
                           (Uint8)(mColor.b),
                           (Uint8)(mColor.a));
    SDL_RenderFillRect(mRenderer, &rect);
}
