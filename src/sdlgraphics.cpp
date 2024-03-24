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

#include "sdlgraphics.h"

#include "log.h"
#include "resources/image.h"
#include "utils/stringutils.h"
#include "video.h"

#include <guichan/exception.hpp>

std::unique_ptr<Graphics> SDLGraphics::create(SDL_Window *window, const VideoSettings &settings)
{
    int rendererFlags = 0;
    if (settings.vsync)
        rendererFlags |= SDL_RENDERER_PRESENTVSYNC;

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, rendererFlags);
    if (!renderer)
    {
        logger->error(strprintf("Failed to create renderer: %s",
                                SDL_GetError()));
        return {};
    }

    return std::make_unique<SDLGraphics>(renderer);
}

SDLGraphics::SDLGraphics(SDL_Renderer *renderer)
    : mRenderer(renderer)
{
    Image::setRenderer(mRenderer);

    SDL_GetRendererOutputSize(mRenderer, &mWidth, &mHeight);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (const char *driver = SDL_GetCurrentVideoDriver())
        logger->log("Using video driver: %s", driver);
    else
        logger->log("Using video driver: not initialized");

    SDL_RendererInfo info;

    if (SDL_GetRendererInfo(renderer, &info) == 0) {
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
}

SDLGraphics::~SDLGraphics()
{
    SDL_DestroyRenderer(mRenderer);
}

void SDLGraphics::setVSync(bool sync)
{
#if SDL_VERSION_ATLEAST(2, 0, 18)
    SDL_RenderSetVSync(mRenderer, sync ? SDL_TRUE : SDL_FALSE);
#endif
}

void SDLGraphics::updateSize(int windowWidth, int windowHeight, float scale)
{
    SDL_GetRendererOutputSize(mRenderer, &mWidth, &mHeight);

    float displayScaleX = windowWidth > 0 ? static_cast<float>(mWidth) / windowWidth : 1.0f;
    float displayScaleY = windowHeight > 0 ? static_cast<float>(mHeight) / windowHeight : 1.0f;

    float scaleX = scale * displayScaleX;
    float scaleY = scale * displayScaleY;

    mWidth = std::ceil(mWidth / scaleX);
    mHeight = std::ceil(mHeight / scaleY);

    SDL_RenderSetScale(mRenderer, scaleX, scaleY);
}

bool SDLGraphics::drawRescaledImage(Image *image,
                                    int srcX, int srcY,
                                    int dstX, int dstY,
                                    int width, int height,
                                    int desiredWidth, int desiredHeight,
                                    bool useColor)
{
    // Check that preconditions for blitting are met.
    if (!image || !image->mTexture)
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

#if SDL_VERSION_ATLEAST(2, 0, 10)
bool SDLGraphics::drawRescaledImageF(Image *image,
                                     int srcX, int srcY,
                                     float dstX, float dstY,
                                     int width, int height,
                                     float desiredWidth, float desiredHeight,
                                     bool useColor)
{
    // Check that preconditions for blitting are met.
    if (!image || !image->mTexture)
        return false;

    dstX += mClipStack.top().xOffset;
    dstY += mClipStack.top().yOffset;

    srcX += image->mBounds.x;
    srcY += image->mBounds.y;

    SDL_FRect dstRect;
    SDL_Rect srcRect;
    dstRect.x = dstX; dstRect.y = dstY;
    srcRect.x = srcX; srcRect.y = srcY;
    srcRect.w = width;
    srcRect.h = height;
    dstRect.w = desiredWidth;
    dstRect.h = desiredHeight;

    return !(SDL_RenderCopyF(mRenderer, image->mTexture, &srcRect, &dstRect) < 0);
}
#endif

void SDLGraphics::drawRescaledImagePattern(Image *image,
                                           int x, int y,
                                           int w, int h,
                                           int scaledWidth,
                                           int scaledHeight)
{
    // Check that preconditions for blitting are met.
    if (!image || !image->mTexture)
        return;

    if (scaledHeight <= 0 || scaledWidth <= 0)
        return;

    SDL_Rect srcRect;
    srcRect.x = image->mBounds.x;
    srcRect.y = image->mBounds.y;

    for (int py = 0; py < h; py += scaledHeight)    // Y position on pattern plane
    {
        int dh = (py + scaledHeight >= h) ? h - py : scaledHeight;
        int dstY = y + py + mClipStack.top().yOffset;

        for (int px = 0; px < w; px += scaledWidth) // X position on pattern plane
        {
            int dw = (px + scaledWidth >= w) ? w - px : scaledWidth;
            int dstX = x + px + mClipStack.top().xOffset;

            SDL_Rect dstRect;
            dstRect.x = dstX; dstRect.y = dstY;
            dstRect.w = dw;   dstRect.h = dh;
            srcRect.w = dw;   srcRect.h = dh;

            if (SDL_RenderCopy(mRenderer, image->mTexture, &srcRect, &dstRect))
                return;
        }
    }
}

void SDLGraphics::updateScreen()
{
    SDL_RenderPresent(mRenderer);
}

void SDLGraphics::windowToLogical(int windowX, int windowY,
                                  float &logicalX, float &logicalY) const
{
#if SDL_VERSION_ATLEAST(2, 0, 18)
    SDL_RenderWindowToLogical(mRenderer, windowX, windowY, &logicalX, &logicalY);
#else
    float scaleX;
    float scaleY;
    SDL_RenderGetScale(mRenderer, &scaleX, &scaleY);
    logicalX = windowX / scaleX;
    logicalY = windowY / scaleY;
#endif
}

SDL_Surface *SDLGraphics::getScreenshot()
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

    SDL_RenderReadPixels(mRenderer, nullptr,
                         screenshot->format->format,
                         screenshot->pixels,
                         screenshot->pitch);

    return screenshot;
}

bool SDLGraphics::pushClipArea(gcn::Rectangle area)
{
    bool result = Graphics::pushClipArea(area);
    updateSDLClipRect();
    return result;
}

void SDLGraphics::popClipArea()
{
    Graphics::popClipArea();
    updateSDLClipRect();
}

void SDLGraphics::updateSDLClipRect()
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

void SDLGraphics::drawPoint(int x, int y)
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

void SDLGraphics::drawLine(int x1, int y1, int x2, int y2)
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

void SDLGraphics::drawRectangle(const gcn::Rectangle &rectangle)
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

void SDLGraphics::fillRectangle(const gcn::Rectangle &rectangle)
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
