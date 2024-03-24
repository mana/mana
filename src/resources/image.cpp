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

#include "resources/image.h"

#include "resources/dye.h"
#include "resources/resourcemanager.h"

#ifdef USE_OPENGL
#include "openglgraphics.h"
#endif

#include "log.h"

#include <SDL_image.h>

#ifdef USE_OPENGL
bool Image::mUseOpenGL = false;
bool Image::mPowerOfTwoTextures = true;
int Image::mTextureType = 0;
int Image::mTextureSize = 0;
#endif

// The low CPU mode is disabled per default
bool Image::mDisableTransparency = false;

SDL_Renderer *Image::mRenderer;

Image::Image(SDL_Texture *texture, int width, int height):
    mTexture(texture)
{
    mBounds.x = 0;
    mBounds.y = 0;
    mBounds.w = width;
    mBounds.h = height;

    if (!texture)
    {
        logger->log(
          "Image::Image(SDL_Texture*, ...): Couldn't load invalid Surface!");
    }
}

#ifdef USE_OPENGL
Image::Image(GLuint glimage, int width, int height, int texWidth, int texHeight):
    mGLImage(glimage),
    mTexWidth(texWidth),
    mTexHeight(texHeight)
{
    mBounds.x = 0;
    mBounds.y = 0;
    mBounds.w = width;
    mBounds.h = height;

    if (glimage == 0)
    {
        logger->log(
          "Image::Image(GLuint, ...): Couldn't load invalid Surface!");
    }
}
#endif

Image::~Image()
{
    if (mTexture)
    {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
    }

#ifdef USE_OPENGL
    if (mGLImage)
    {
        glDeleteTextures(1, &mGLImage);
        mGLImage = 0;
    }
#endif
}

Resource *Image::load(SDL_RWops *rw)
{
    SDL_Surface *tmpImage = IMG_Load_RW(rw, 1);

    if (!tmpImage)
    {
        logger->log("Error, image load failed: %s", IMG_GetError());
        return nullptr;
    }

    Image *image = load(tmpImage);

    SDL_FreeSurface(tmpImage);
    return image;
}

Resource *Image::load(SDL_RWops *rw, const Dye &dye)
{
    SDL_Surface *tmpImage = IMG_Load_RW(rw, 1);

    if (!tmpImage)
    {
        logger->log("Error, image load failed: %s", IMG_GetError());
        return nullptr;
    }

    SDL_PixelFormat rgba;
    rgba.palette = nullptr;
    rgba.BitsPerPixel = 32;
    rgba.BytesPerPixel = 4;
    rgba.Rmask = 0xFF000000; rgba.Rloss = 0; rgba.Rshift = 24;
    rgba.Gmask = 0x00FF0000; rgba.Gloss = 0; rgba.Gshift = 16;
    rgba.Bmask = 0x0000FF00; rgba.Bloss = 0; rgba.Bshift = 8;
    rgba.Amask = 0x000000FF; rgba.Aloss = 0; rgba.Ashift = 0;

    SDL_Surface *surf = SDL_ConvertSurface(tmpImage, &rgba, 0);
    SDL_FreeSurface(tmpImage);

    auto *pixels = static_cast< Uint32 * >(surf->pixels);
    for (Uint32 *p_end = pixels + surf->w * surf->h; pixels != p_end; ++pixels)
    {
        int alpha = *pixels & 255;
        if (!alpha) continue;
        int v[3];
        v[0] = (*pixels >> 24) & 255;
        v[1] = (*pixels >> 16) & 255;
        v[2] = (*pixels >> 8 ) & 255;
        dye.update(v);
        *pixels = (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | alpha;
    }

    Image *image = load(surf);
    SDL_FreeSurface(surf);
    return image;
}

Image *Image::load(SDL_Surface *tmpImage)
{
#ifdef USE_OPENGL
    if (mUseOpenGL)
        return _GLload(tmpImage);
#endif
    return _SDLload(tmpImage);
}

bool Image::useOpenGL()
{
#ifdef USE_OPENGL
    return mUseOpenGL;
#else
    return false;
#endif
}

void Image::setAlpha(float alpha)
{
    if (!useOpenGL() && mDisableTransparency)
        return;

    if (mAlpha == alpha)
        return;

    if (alpha < 0.0f || alpha > 1.0f)
        return;

    mAlpha = alpha;

    if (mTexture)
    {
        SDL_SetTextureAlphaMod(mTexture, (Uint8) (255 * mAlpha));
    }
}

Image *Image::_SDLload(SDL_Surface *image)
{
    if (!image || !mRenderer)
        return nullptr;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(mRenderer, image);
    return new Image(texture, image->w, image->h);
}

void Image::setRenderer(SDL_Renderer *renderer)
{
    mRenderer = renderer;
}

#ifdef USE_OPENGL
Image *Image::_GLload(SDL_Surface *image)
{
    // Flush current error flag.
    glGetError();

    int width = image->w;
    int height = image->h;
    int realWidth = powerOfTwo(width);
    int realHeight = powerOfTwo(height);

    if (realWidth < width || realHeight < height)
    {
        logger->log("Warning: image too large, cropping to %dx%d texture!",
                    realWidth, realHeight);
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

    bool needsConversion = !(realWidth == width &&
                             realHeight == height &&
                             image->format->BytesPerPixel == 4 &&
                             image->format->Rmask == rmask &&
                             image->format->Gmask == gmask &&
                             image->format->Bmask == bmask &&
                             image->format->Amask == amask);

    if (needsConversion)
    {
        SDL_Surface *oldImage = image;
        image = SDL_CreateRGBSurface(SDL_SWSURFACE, realWidth, realHeight,
                                     32, rmask, gmask, bmask, amask);

        if (!image)
        {
            logger->log("Error, image convert failed: out of memory");
            return nullptr;
        }

        // Make sure the alpha channel is not used, but copied to destination
        SDL_SetSurfaceBlendMode(oldImage, SDL_BLENDMODE_NONE);
        SDL_BlitSurface(oldImage, nullptr, image, nullptr);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    OpenGLGraphics::bindTexture(mTextureType, texture);

    if (SDL_MUSTLOCK(image))
        SDL_LockSurface(image);

    glTexImage2D(mTextureType, 0, GL_RGBA8,
                 image->w, image->h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image->pixels);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(mTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(mTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (SDL_MUSTLOCK(image))
        SDL_UnlockSurface(image);

    if (needsConversion)
        SDL_FreeSurface(image);

    if (GLenum error = glGetError())
    {
        const char *errmsg = "Unknown error";
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
        logger->log("Error: Image GL import failed: %s", errmsg);
        return nullptr;
    }

    return new Image(texture, width, height, realWidth, realHeight);
}

void Image::setLoadAsOpenGL(bool useOpenGL)
{
    Image::mUseOpenGL = useOpenGL;
}

int Image::powerOfTwo(int input)
{
    int value;
    if (mPowerOfTwoTextures)
    {
        value = 1;
        while (value < input && value < mTextureSize)
        {
            value <<= 1;
        }
    }
    else
    {
        value = input;
    }
    return value >= mTextureSize ? mTextureSize : value;
}
#endif

Image *Image::getSubImage(int x, int y, int width, int height)
{
    // Create a new clipped sub-image
#ifdef USE_OPENGL
    if (mUseOpenGL)
        return new SubImage(this, mGLImage,
                            mBounds.x + x,
                            mBounds.y + y,
                            width, height,
                            mTexWidth, mTexHeight);
#endif

    return new SubImage(this, mTexture,
                        mBounds.x + x,
                        mBounds.y + y,
                        width, height);
}

//============================================================================
// SubImage Class
//============================================================================

SubImage::SubImage(Image *parent, SDL_Texture *texture,
                   int x, int y, int width, int height):
    Image(texture, width, height),
    mParent(parent)
{
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
    Image(image, width, height, texWidth, texHeight),
    mParent(parent)
{
    // Set up the rectangle.
    mBounds.x = x;
    mBounds.y = y;
    mBounds.w = width;
    mBounds.h = height;
}
#endif

SubImage::~SubImage()
{
    // Avoid destruction of the texture
    mTexture = nullptr;
#ifdef USE_OPENGL
    mGLImage = 0;
#endif
}
