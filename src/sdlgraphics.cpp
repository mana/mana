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

#include <cmath>

std::unique_ptr<Graphics> SDLGraphics::create(SDL_Window *window, const VideoSettings &settings)
{
    int rendererFlags = 0;
    if (settings.vsync)
        rendererFlags |= SDL_RENDERER_PRESENTVSYNC;

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, rendererFlags);
    if (!renderer)
    {
        Log::critical(strprintf("Failed to create renderer: %s",
                                SDL_GetError()));
        return {};
    }

    return std::make_unique<SDLGraphics>(window, renderer);
}

SDLGraphics::SDLGraphics(SDL_Window *window, SDL_Renderer *renderer)
    : mWindow(window)
    , mRenderer(renderer)
{
    Image::setRenderer(mRenderer);

    SDL_GetRendererOutputSize(mRenderer, &mWidth, &mHeight);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (const char *driver = SDL_GetCurrentVideoDriver())
        Log::info("Using video driver: %s", driver);
    else
        Log::info("Using video driver: not initialized");

    SDL_RendererInfo info;

    if (SDL_GetRendererInfo(renderer, &info) == 0) {
        Log::info("Using renderer: %s", info.name);

        Log::info("The renderer is a software fallback: %s",
                  (info.flags & SDL_RENDERER_SOFTWARE) ? "yes" : "no");
        Log::info("The renderer is hardware accelerated: %s",
                  (info.flags & SDL_RENDERER_ACCELERATED) ? "yes" : "no");
        Log::info("Vsync: %s",
                  (info.flags & SDL_RENDERER_PRESENTVSYNC) ? "on" : "off");
        Log::info("Renderer supports rendering to texture: %s",
                  (info.flags & SDL_RENDERER_TARGETTEXTURE) ? "yes" : "no");
        Log::info("Max texture size: %dx%d",
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
    mScale = scaleX;

    SDL_RenderSetScale(mRenderer, scaleX, scaleY);
}

bool SDLGraphics::drawRescaledImage(const Image *image,
                                    int srcX, int srcY,
                                    int dstX, int dstY,
                                    int width, int height,
                                    int desiredWidth, int desiredHeight)
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

    setColorAlphaMod(image);
    return SDL_RenderCopy(mRenderer, image->mTexture, &srcRect, &dstRect) != 0;
}

#if SDL_VERSION_ATLEAST(2, 0, 10)
bool SDLGraphics::drawRescaledImageF(const Image *image,
                                     int srcX, int srcY,
                                     float dstX, float dstY,
                                     int width, int height,
                                     float desiredWidth, float desiredHeight)
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

    setColorAlphaMod(image);
    return SDL_RenderCopyF(mRenderer, image->mTexture, &srcRect, &dstRect) == 0;
}
#endif

void SDLGraphics::drawRescaledImagePattern(const Image *image,
                                           int srcX, int srcY,
                                           int srcW, int srcH,
                                           int dstX, int dstY,
                                           int dstW, int dstH,
                                           int scaledWidth,
                                           int scaledHeight)
{
    // Check that preconditions for blitting are met.
    if (!image || !image->mTexture)
        return;

    if (scaledHeight <= 0 || scaledWidth <= 0)
        return;

    setColorAlphaMod(image);

    SDL_Rect srcRect;
    srcRect.x = image->mBounds.x + srcX;
    srcRect.y = image->mBounds.y + srcY;

    for (int py = 0; py < dstH; py += scaledHeight)    // Y position on pattern plane
    {
        SDL_Rect dstRect;
        dstRect.h = (py + scaledHeight >= dstH) ? dstH - py : scaledHeight;
        dstRect.y = dstY + py + mClipStack.top().yOffset;

        for (int px = 0; px < dstW; px += scaledWidth) // X position on pattern plane
        {
            dstRect.x = dstX + px + mClipStack.top().xOffset;
            dstRect.w = (px + scaledWidth >= dstW) ? dstW - px : scaledWidth;

            srcRect.w = srcW * dstRect.w / scaledWidth;
            srcRect.h = srcH * dstRect.h / scaledHeight;

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

    int width, height;
    if (SDL_GetRendererOutputSize(mRenderer, &width, &height) != 0)
        return nullptr;

    SDL_Surface *screenshot = SDL_CreateRGBSurface(0, width, height, 24,
                                                   rmask, gmask, bmask, amask);

    if (SDL_RenderReadPixels(mRenderer, nullptr,
                             screenshot->format->format,
                             screenshot->pixels,
                             screenshot->pitch) != 0)
    {
        SDL_FreeSurface(screenshot);
        screenshot = nullptr;
    }

    return screenshot;
}

void SDLGraphics::updateClipRect()
{
    if (mClipRects.empty())
    {
        SDL_RenderSetClipRect(mRenderer, nullptr);
        return;
    }

    const gcn::Rectangle &clipRect = mClipRects.top();
    const SDL_Rect rect = {
        clipRect.x,
        clipRect.y,
        clipRect.width,
        clipRect.height
    };

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

void SDLGraphics::setColorAlphaMod(const Image *image) const
{
    Uint8 r = 255, g = 255, b = 255, a = 255;
    if (image->useColor())
    {
        r = static_cast<Uint8>(mColor.r);
        g = static_cast<Uint8>(mColor.g);
        b = static_cast<Uint8>(mColor.b);
        a = static_cast<Uint8>(mColor.a);
    }

    SDL_Texture *texture = image->mTexture;
    SDL_SetTextureColorMod(texture, r, g, b);
    SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(a * image->getAlpha()));
}
