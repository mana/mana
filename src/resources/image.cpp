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

#include "image.h"

#include <SDL_image.h>

#include "../log.h"

#ifdef USE_OPENGL
bool Image::mUseOpenGL = false;
#endif

Image::Image(const std::string &idPath, SDL_Surface *image):
    Resource(idPath), mImage(image),
    mAlpha(1.0f)
{
    mBounds.x = 0;
    mBounds.y = 0;
    mBounds.w = mImage->w;
    mBounds.h = mImage->h;
}

#ifdef USE_OPENGL
Image::Image(const std::string &idPath, GLuint glimage, int width, int height,
             int texWidth, int texHeight):
    Resource(idPath),
    mGLImage(glimage),
    mTexWidth(texWidth),
    mTexHeight(texHeight),
    mImage(0),
    mAlpha(1.0)
{
    mBounds.x = 0;
    mBounds.y = 0;
    mBounds.w = width;
    mBounds.h = height;
}
#endif

Image::~Image()
{
    unload();
}

Image* Image::load(void *buffer, unsigned int bufferSize,
                   const std::string &idPath)
{
    // Load the raw file data from the buffer in an RWops structure
    SDL_RWops *rw = SDL_RWFromMem(buffer, bufferSize);
    SDL_Surface *tmpImage;

    // Use SDL_Image to load the raw image data and have it free the data
    if (!idPath.compare(idPath.length() - 4, 4, ".tga"))
    {
        tmpImage = IMG_LoadTyped_RW(rw, 1, const_cast<char*>("TGA"));
    }
    else
    {
        tmpImage = IMG_Load_RW(rw, 1);
    }

    if (!tmpImage) {
        logger->log("Error, image load failed: %s", IMG_GetError());
        return NULL;
    }

    // Determine 32-bit masks based on byte order
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

#ifdef USE_OPENGL
    if (mUseOpenGL)
    {
        int width = tmpImage->w;
        int height = tmpImage->h;
        int realWidth = powerOfTwo(width);
        int realHeight = powerOfTwo(height);

        if (realWidth < width || realHeight < height)
        {
            logger->log("Warning: image too large, cropping to %dx%d texture!",
                    tmpImage->w, tmpImage->h);
        }

        // Make sure the alpha channel is not used, but copied to destination
        SDL_SetAlpha(tmpImage, 0, SDL_ALPHA_OPAQUE);

        SDL_Surface *oldImage = tmpImage;
        tmpImage = SDL_CreateRGBSurface(SDL_SWSURFACE, realWidth, realHeight,
            32, rmask, gmask, bmask, amask);

        if (!tmpImage) {
            logger->log("Error, image convert failed: out of memory");
            return NULL;
        }

        SDL_BlitSurface(oldImage, NULL, tmpImage, NULL);
        SDL_FreeSurface(oldImage);

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        if (SDL_MUSTLOCK(tmpImage)) {
            SDL_LockSurface(tmpImage);
        }

        glTexImage2D(
                GL_TEXTURE_2D, 0, 4,
                tmpImage->w, tmpImage->h,
                0, GL_RGBA, GL_UNSIGNED_BYTE,
                tmpImage->pixels);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (SDL_MUSTLOCK(tmpImage)) {
            SDL_UnlockSurface(tmpImage);
        }

        SDL_FreeSurface(tmpImage);

        GLenum error = glGetError();
        if (error)
        {
            std::string errmsg = "Unknown error";
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
            logger->log("Error: Image GL import failed: %s", errmsg.c_str());
            return NULL;
        }

        return new Image(idPath, texture, width, height, realWidth, realHeight);
    }
#endif

    bool hasAlpha = false;

    // Figure out whether the image uses its alpha layer
    for (int i = 0; i < tmpImage->w * tmpImage->h; ++i)
    {
        Uint8 r, g, b, a;
        SDL_GetRGBA(
                ((Uint32*) tmpImage->pixels)[i],
                tmpImage->format,
                &r, &g, &b, &a);

        if (a != 255)
        {
            hasAlpha = true;
            break;
        }
    }

    SDL_Surface *image;

    // Convert the surface to the current display format
    if (hasAlpha) {
        image = SDL_DisplayFormatAlpha(tmpImage);
    }
    else {
        image = SDL_DisplayFormat(tmpImage);
    }
    SDL_FreeSurface(tmpImage);

    if (!image) {
        logger->log("Error: Image convert failed.");
        return NULL;
    }

    return new Image(idPath, image);
}

void Image::unload()
{
    mLoaded = false;

    if (mImage) {
        // Free the image surface.
        SDL_FreeSurface(mImage);
        mImage = NULL;
    }

#ifdef USE_OPENGL
    if (mGLImage) {
        glDeleteTextures(1, &mGLImage);
        mGLImage = 0;
    }
#endif
}

Image *Image::getSubImage(int x, int y, int width, int height)
{
    // Create a new clipped sub-image
#ifdef USE_OPENGL
    if (mUseOpenGL) {
        return new SubImage(this, mGLImage, x, y, width, height,
                            mTexWidth, mTexHeight);
    }
#endif

    return new SubImage(this, mImage, x, y, width, height);
}

void Image::setAlpha(float a)
{
    if (mAlpha == a) {
        return;
    }

    mAlpha = a;

    if (mImage) {
        // Set the alpha value this image is drawn at
        SDL_SetAlpha(mImage, SDL_SRCALPHA, (int) (255 * mAlpha));
    }
}

float Image::getAlpha()
{
    return mAlpha;
}

#ifdef USE_OPENGL
void
Image::setLoadAsOpenGL(bool useOpenGL)
{
    Image::mUseOpenGL = useOpenGL;
}

int
Image::powerOfTwo(int input)
{
    int value = 1;
    while (value < input && value < 1024)
    {
        value <<= 1;
    }
    return value;
}
#endif

//============================================================================
// SubImage Class
//============================================================================

SubImage::SubImage(Image *parent, SDL_Surface *image,
        int x, int y, int width, int height):
    Image("", image), mParent(parent)
{
    mParent->incRef();

    // Set up the rectangle.
    mBounds.x = x;
    mBounds.y = y;
    mBounds.w = width;
    mBounds.h = height;
}

#ifdef USE_OPENGL
SubImage::SubImage(Image *parent, GLuint image,
                   int x, int y, int width, int height,
                   int texWidth, int texHeight):
    Image("", image, width, height, texWidth, texHeight), mParent(parent)
{
    mParent->incRef();

    // Set up the rectangle.
    mBounds.x = x;
    mBounds.y = y;
    mBounds.w = width;
    mBounds.h = height;
}
#endif

SubImage::~SubImage()
{
    // Avoid destruction of the image
    mImage = 0;
#ifdef USE_OPENGL
    mGLImage = 0;
#endif
    mParent->decRef();
}

Image *SubImage::getSubImage(int x, int y, int w, int h)
{
    return NULL;
}
