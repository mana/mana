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

#include "../log.h"
#include "image.h"
#include <iostream>
#include <SDL_image.h>
#include "../graphic/graphic.h"
#include "resourcemanager.h"

Image::Image(SDL_Surface *image):
    image(image)
{
}

Image::~Image()
{
    unload();
}

Image* Image::load(const std::string &filePath, int flags)
{
    std::stringstream msg;
    msg << "Image::load(" << filePath << ")";
    log(msg.str());

    // Attempt to use SDL_Image to load the file.
    SDL_Surface *tmpImage = IMG_Load(filePath.c_str());
    SDL_SetColorKey(tmpImage, SDL_SRCCOLORKEY | SDL_RLEACCEL,
            SDL_MapRGB(tmpImage->format, 255, 0, 255));
    SDL_Surface *image = NULL;
    if (flags & IMG_ALPHA) {
        image = SDL_DisplayFormatAlpha(tmpImage);
    }
    else {
        image = SDL_DisplayFormat(tmpImage);
    }
    SDL_FreeSurface(tmpImage);

    // Check if the file was opened and return the appropriate value.
    if (!image) {
        log("Error", "Image load failed : %s", IMG_GetError());
        //log("Error", "Image load failed : %s", filePath.c_str());
        return NULL;
    }

    return new Image(image);
}

void Image::unload()
{
    // Free the image surface.
    if (image != NULL) {
        SDL_FreeSurface(image);
        image = NULL;
        loaded = false;
    }
}

int Image::getWidth() const
{
    if (image != NULL) {
        return image->w;
    }
    return 0;
}

int Image::getHeight() const
{
    if (image != NULL) {
        return image->h;
    }
    return 0;
}

Image *Image::getSubImage(int x, int y, int width, int height)
{
    // Create a new clipped sub-image
    return new SubImage(this, image, x, y, width, height);
}

Image *Image::getScaledInstance(int width, int height)
{
    return new ScaledImage(this, image, width, height);
}

bool Image::draw(SDL_Surface *screen, int srcX, int srcY, int dstX, int dstY,
        int width, int height)
{
    // Check that preconditions for blitting are met.
    if (screen == NULL || image == NULL) return false;

    SDL_Rect dstRect;
    SDL_Rect srcRect;
    dstRect.x = dstX; dstRect.y = dstY;
    srcRect.x = srcX; srcRect.y = srcY;
    srcRect.w = width;
    srcRect.h = height;

    if (SDL_BlitSurface(image, &srcRect, screen, &dstRect) < 0) {
        return false;
    }

    return true;
}

bool Image::draw(SDL_Surface *screen, int x, int y)
{
    // Check that preconditions for blitting are met.
    if (screen == NULL || image == NULL) return false;

    SDL_Rect dstRect;
    dstRect.x = x;
    dstRect.y = y;

    if (SDL_BlitSurface(image, NULL, screen, &dstRect) < 0) {
        return false;
    }

    return true;
}

void Image::drawPattern(SDL_Surface *screen, int x, int y, int w, int h)
{
    int iw = getWidth();              // Width of image
    int ih = getHeight();             // Height of image
    if (iw == 0 || ih == 0) return;

    int px = 0;                       // X position on pattern plane
    int py = 0;                       // Y position on pattern plane

    while (py < h) {
        while (px < w) {
            int dw = (px + iw >= w) ? w - px : iw;
            int dh = (py + ih >= h) ? h - py : ih;
            draw(screen, 0, 0, x + px, y + py, dw, dh);
            px += iw;
        }
        py += ih;
        px = 0;
    }
}

//============================================================================

SubImage::SubImage(Image *parent, SDL_Surface *image,
        int x, int y, int width, int height):
    Image(image),
    parent(parent)
{
    parent->incRef();

    // Set up the rectangle.
    rect.x = x;
    rect.y = y;
    rect.w = width;
    rect.h = height;
}

SubImage::~SubImage()
{
    image = NULL;
    // TODO: Enable when no longer a problem
    //parent->decRef();
}

int SubImage::getWidth() const
{
    return rect.w;
}

int SubImage::getHeight() const
{
    return rect.h;
}

Image *SubImage::getSubImage(int x, int y, int w, int h)
{
    return NULL;
}

bool SubImage::draw(SDL_Surface *screen, int srcX, int srcY,
        int dstX, int dstY, int width, int height)
{
    // Check that preconditions for blitting are met.
    if (screen == NULL || image == NULL) return false;

    SDL_Rect dstRect;
    SDL_Rect srcRect;
    dstRect.x = dstX; dstRect.y = dstY;
    srcRect.x = rect.x + srcX;
    srcRect.y = rect.y + srcY;
    srcRect.w = width;
    srcRect.h = height;

    if (SDL_BlitSurface(image, &srcRect, screen, &dstRect) < 0) {
        return false;
    }

    return true;
}

bool SubImage::draw(SDL_Surface *screen, int x, int y)
{
    // Check that drawing preconditions are satisfied.
    if (screen == NULL || image == NULL) return false;

    SDL_Rect dstRect;
    dstRect.x = x;
    dstRect.y = y;

    // Draw subimage part to given location
    if (SDL_BlitSurface(image, &rect, screen, &dstRect) < 0) {
        return false;
    }

    return true;
}

//============================================================================

ScaledImage::ScaledImage(
        Image *parent, SDL_Surface *bmp, int width, int height):
    Image(SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                0xff000000,
                0x00ff0000,
                0x0000ff00,
                0x000000ff
#else
                0x000000ff,
                0x0000ff00,
                0x00ff0000,
                0xff000000
#endif
                ))
{
    if (image) {
        // Somehow stretch the image using SDL_gfx
        //stretch_blit(bmp, image, 0, 0, bmp->w, bmp->h, 0, 0, width, height);
    }
}

ScaledImage::~ScaledImage()
{
    if (image) {
        SDL_FreeSurface(image);
        image = NULL;
    }
}
