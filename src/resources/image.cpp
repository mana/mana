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

#include "../main.h"
#include "../graphics.h"
#include "image.h"
#include "../log.h"
#include <iostream>
#include <SDL_image.h>
#include "resourcemanager.h"

#ifndef USE_OPENGL
Image::Image(SDL_Surface *image):
    image(image)
#else
Image::Image(GLuint image, int width, int height, int texWidth, int texHeight):
    image(image),
    width(width),
    height(height),
    texWidth(texWidth),
    texHeight(texHeight)
#endif
{
    // Default to opaque
    alpha = 1.0f;
}

Image::~Image()
{
    unload();
}

Image* Image::load(void* buffer, unsigned int bufferSize, int flags)
{
    // Load the raw file data from the buffer in an RWops structure
    SDL_RWops *rw = SDL_RWFromMem(buffer, bufferSize);

    // Use SDL_Image to load the raw image data
    SDL_Surface* tmpImage = IMG_Load_RW(rw, 1);

    // Now free the SDL_RWops data
    //SDL_FreeRW(rw);

#ifndef USE_OPENGL

    SDL_Surface *image;
    if (flags & IMG_ALPHA) {
        image = SDL_DisplayFormatAlpha(tmpImage);
    }
    else {
        SDL_SetColorKey(tmpImage, SDL_SRCCOLORKEY | SDL_RLEACCEL,
                SDL_MapRGB(tmpImage->format, 255, 0, 255));
        image = SDL_DisplayFormat(tmpImage);
    }
    SDL_FreeSurface(tmpImage);

    // Check if the file was opened and return the appropriate value.
    if (!image) {
        logger.log("Error: Image convert failed.");
        return NULL;
    }

    return new Image(image);

#else

    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    SDL_Surface *formatImage = SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, 32,
            rmask, gmask, bmask, amask);

    if (formatImage == NULL) {
        logger.log("Error", "Image load failed: not enough memory");
    }

    SDL_Surface *image = SDL_ConvertSurface(
            tmpImage, formatImage->format, SDL_SWSURFACE);
    SDL_FreeSurface(formatImage);
    SDL_FreeSurface(tmpImage);

    unsigned int *rawData = (unsigned int *)image->pixels;
    int width = image->w;
    int height = image->h;
    int realWidth = 1, realHeight = 1;

    while (realWidth < width && realWidth < 1024) {
        realWidth *= 2;
    }

    while (realHeight < height && realHeight < 1024) {
        realHeight *= 2;
    }

    unsigned int *realData = new unsigned int[realWidth * realHeight];
    int x, y;

    for (y = 0; y < realHeight; y++)
    {
        for (x = 0; x < realWidth; x++)
        {
            if (x < width && y < height)
            {
                if (rawData[x + y * width] == 0xffff00ff)
                {
                    realData[x + y * realWidth] = 0x00000000;
                }
                else
                {
                    realData[x + y * realWidth] = rawData[x + y * width];
                }
            }
            else
            {
                realData[x + y * realWidth] = 0;
            }
        }
    }

    GLuint texture;
    glGenTextures(1, &texture);
    logger.log("Binding texture %d (%dx%d)", texture, realWidth, realHeight);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
            GL_TEXTURE_2D, 0, 4,
            realWidth, realHeight,
            0, GL_RGBA, GL_UNSIGNED_BYTE,
            realData);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    delete[] realData;
    SDL_FreeSurface(image);

    GLenum error = glGetError();
    if (error)
    {
        std::string errmsg = "Unkown error";
        switch (error)
        {
            case GL_INVALID_ENUM:
                errmsg = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                errmsg = "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                errmsg = "GL_INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                errmsg = "GL_STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                errmsg = "GL_STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                errmsg = "GL_OUT_OF_MEMORY";
                break;
        }
        logger.log("Error: Image GL import failed: %s", errmsg.c_str());
        return NULL;
    }

    return new Image(texture, width, height, realWidth, realHeight);
#endif
}

Image *Image::create(int width, int height)
{
#ifndef USE_OPENGL
    SDL_Surface *surf =
        SDL_AllocSurface(SDL_SWSURFACE, width, height, 32, 0, 0, 0, 0);

    if (surf) {
        return new Image(surf);
    }
    else {
        return NULL;
    }
#else
    return NULL;
#endif
}


void Image::unload()
{
    // Free the image surface.
#ifndef USE_OPENGL
    if (image != NULL) {
        SDL_FreeSurface(image);
        image = NULL;
        loaded = false;
    }
#endif
    loaded = false;
}

int Image::getWidth() const
{
#ifndef USE_OPENGL
    if (image != NULL) {
        return image->w;
    }
#else
    return width;
#endif
    return 0;
}

int Image::getHeight() const
{
#ifndef USE_OPENGL
    if (image != NULL) {
        return image->h;
    }
#else
    return height;
#endif
    return 0;
}

Image *Image::getSubImage(int x, int y, int width, int height)
{
    // Create a new clipped sub-image
#ifdef USE_OPENGL
    return new SubImage(this, image, x, y, width, height, texWidth, texHeight);
#else
    return new SubImage(this, image, x, y, width, height);
#endif
}

bool Image::draw(SDL_Surface *screen, int srcX, int srcY, int dstX, int dstY,
        int width, int height)
{
#ifndef USE_OPENGL
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

#else

    // Find OpenGL texture coordinates
    float texX1 = srcX / (float)texWidth;
    float texY1 = srcY / (float)texHeight;
    float texX2 = (srcX + width) / (float)texWidth;
    float texY2 = (srcY + height) / (float)texHeight;

    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBindTexture(GL_TEXTURE_2D, image);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    // Draw a textured quad -- the image
    glBegin(GL_QUADS);
    glTexCoord2f(texX1, texY1);
    glVertex3i(dstX, dstY, 0);

    glTexCoord2f(texX2, texY1);
    glVertex3i(dstX + width, dstY, 0);

    glTexCoord2f(texX2, texY2);
    glVertex3i(dstX + width, dstY + height, 0);

    glTexCoord2f(texX1, texY2);
    glVertex3i(dstX, dstY + height, 0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

#endif
    return true;
}

bool Image::draw(SDL_Surface *screen, int x, int y)
{
    return draw(screen, 0, 0, x, y, getWidth(), getHeight());
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

void Image::setAlpha(float alpha)
{
    this->alpha = alpha;

#ifndef USE_OPENGL
    // Set the alpha value this image is drawn at
    SDL_SetAlpha(image, SDL_SRCALPHA | SDL_RLEACCEL, (int)(255 * alpha));
#endif
}

float Image::getAlpha()
{
    return alpha;
}

void Image::fillWithColor(
        unsigned char red, unsigned char green, unsigned blue)
{
#ifndef USE_OPENGL
   if (image) {
       Uint32 boxColor = SDL_MapRGB(image->format, red, green, blue);
       SDL_Rect sourceRect;
       sourceRect.x = sourceRect.y = 0;
       sourceRect.w = image->w;
       sourceRect.h = image->h;
       SDL_FillRect(image, &sourceRect, boxColor);
   }
#endif
}

//============================================================================
// SubImage Class
//============================================================================

#ifndef USE_OPENGL
SubImage::SubImage(Image *parent, SDL_Surface *image,
        int x, int y, int width, int height):
    Image(image),
#else
SubImage::SubImage(Image *parent, GLuint image,
        int x, int y, int width, int height, int texWidth, int texHeight):
    Image(image, width, height, texWidth, texHeight),
#endif
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
#ifndef USE_OPENGL
    image = NULL;
#endif
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
#ifndef USE_OPENGL
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

#else

    // Find OpenGL texture coordinates
    float texX1 = (rect.x + srcX) / (float)texWidth;
    float texY1 = (rect.y + srcY) / (float)texHeight;
    float texX2 = (rect.x + srcX + width) / (float)texWidth;
    float texY2 = (rect.y + srcY + height) / (float)texHeight;

    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBindTexture(GL_TEXTURE_2D, image);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    // Draw a textured quad -- the image
    glBegin(GL_QUADS);
    glTexCoord2f(texX1, texY1);
    glVertex3i(dstX, dstY, 0);

    glTexCoord2f(texX2, texY1);
    glVertex3i(dstX + width, dstY, 0);

    glTexCoord2f(texX2, texY2);
    glVertex3i(dstX + width, dstY + height, 0);

    glTexCoord2f(texX1, texY2);
    glVertex3i(dstX, dstY + height, 0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

#endif
    return true;
}

bool SubImage::draw(SDL_Surface *screen, int x, int y)
{
    return draw(screen, 0, 0, x, y, getWidth(), getHeight());
}
