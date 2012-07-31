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
#include "configuration.h"

#include <SDL_image.h>
#include <SDL_rotozoom.h>

#ifdef USE_OPENGL
bool Image::mUseOpenGL = false;
bool Image::mPowerOfTwoTextures = true;
int Image::mTextureType = 0;
int Image::mTextureSize = 0;
#endif
bool Image::mEnableAlphaCache = false;

// The low CPU mode is disabled per default
bool Image::mDisableTransparency = false;

Image::Image(SDL_Surface *image, bool hasAlphaChannel, Uint8 *alphaChannel):
    mAlpha(1.0f),
    mSDLSurface(image),
    mAlphaChannel(alphaChannel),
    mHasAlphaChannel(hasAlphaChannel)
{
#ifdef USE_OPENGL
    mGLImage = 0;
#endif

    mUseAlphaCache = Image::mEnableAlphaCache;

    mBounds.x = 0;
    mBounds.y = 0;

    mLoaded = false;

    if (mSDLSurface)
    {
        mBounds.w = mSDLSurface->w;
        mBounds.h = mSDLSurface->h;

        mLoaded = true;
    }
    else
        logger->log(
          "Image::Image(SDL_Surface*): Couldn't load invalid Surface!");
}

#ifdef USE_OPENGL
Image::Image(GLuint glimage, int width, int height, int texWidth, int texHeight):
    mAlpha(1.0f),
    mSDLSurface(0),
    mAlphaChannel(0),
    mHasAlphaChannel(true),
    mUseAlphaCache(false),
    mGLImage(glimage),
    mTexWidth(texWidth),
    mTexHeight(texHeight)
{
    mBounds.x = 0;
    mBounds.y = 0;
    mBounds.w = width;
    mBounds.h = height;

    if (mGLImage)
        mLoaded = true;
    else
    {
        logger->log(
          "Image::Image(GLuint*, ...): Couldn't load invalid Surface!");
        mLoaded = false;
    }
}
#endif

Image::~Image()
{
    unload();
}

Resource *Image::load(SDL_RWops *rw)
{
    SDL_Surface *tmpImage = IMG_Load_RW(rw, 1);

    if (!tmpImage)
    {
        logger->log("Error, image load failed: %s", IMG_GetError());
        return NULL;
    }

    Image *image = load(tmpImage);

    SDL_FreeSurface(tmpImage);
    return image;
}

Resource *Image::load(SDL_RWops *rw, Dye const &dye)
{
    SDL_Surface *tmpImage = IMG_Load_RW(rw, 1);

    if (!tmpImage)
    {
        logger->log("Error, image load failed: %s", IMG_GetError());
        return NULL;
    }

    SDL_PixelFormat rgba;
    rgba.palette = NULL;
    rgba.BitsPerPixel = 32;
    rgba.BytesPerPixel = 4;
    rgba.Rmask = 0xFF000000; rgba.Rloss = 0; rgba.Rshift = 24;
    rgba.Gmask = 0x00FF0000; rgba.Gloss = 0; rgba.Gshift = 16;
    rgba.Bmask = 0x0000FF00; rgba.Bloss = 0; rgba.Bshift = 8;
    rgba.Amask = 0x000000FF; rgba.Aloss = 0; rgba.Ashift = 0;
    rgba.colorkey = 0;
    rgba.alpha = 255;

    SDL_Surface *surf = SDL_ConvertSurface(tmpImage, &rgba, SDL_SWSURFACE);
    SDL_FreeSurface(tmpImage);

    Uint32 *pixels = static_cast< Uint32 * >(surf->pixels);
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

void Image::SDLcleanCache()
{
    ResourceManager *resman = ResourceManager::getInstance();

    for (std::map<float, SDL_Surface*>::iterator
         i = mAlphaCache.begin(), i_end = mAlphaCache.end();
         i != i_end; ++i)
    {
        if (mSDLSurface != i->second)
            resman->scheduleDelete(i->second);
        i->second = 0;
    }
    mAlphaCache.clear();
}

void Image::unload()
{
    mLoaded = false;

    if (mSDLSurface)
    {
        SDLcleanCache();
        // Free the image surface.
        SDL_FreeSurface(mSDLSurface);
        mSDLSurface = NULL;

        delete[] mAlphaChannel;
        mAlphaChannel = NULL;
    }

#ifdef USE_OPENGL
    if (mGLImage)
    {
        glDeleteTextures(1, &mGLImage);
        mGLImage = 0;
    }
#endif
}

bool Image::useOpenGL()
{
#ifdef USE_OPENGL
    return mUseOpenGL;
#else
    return false;
#endif
}

bool Image::hasAlphaChannel()
{
    if (!mLoaded)
        return false;

#ifdef USE_OPENGL
    if (mUseOpenGL)
        return true;
#endif

    return mHasAlphaChannel;
}

SDL_Surface *Image::getByAlpha(float alpha)
{
    std::map<float, SDL_Surface*>::iterator it = mAlphaCache.find(alpha);
    if (it != mAlphaCache.end())
        return (*it).second;
    return 0;
}

void Image::setAlpha(float alpha)
{
    if (!useOpenGL() && mDisableTransparency)
        return;

    if (mAlpha == alpha)
        return;

    if (alpha < 0.0f || alpha > 1.0f)
        return;

    if (mSDLSurface)
    {
        if (mUseAlphaCache)
        {
            SDL_Surface *surface = getByAlpha(mAlpha);
            if (!surface)
            {
                if (mAlphaCache.size() > 100)
                    SDLcleanCache();

                mAlphaCache[mAlpha] = mSDLSurface;
            }
            surface = getByAlpha(alpha);
            if (surface)
            {
                mAlphaCache.erase(alpha);
                mSDLSurface = surface;
                mAlpha = alpha;
                return;
            }
            else
            {
                mSDLSurface = Image::SDLduplicateSurface(mSDLSurface);
            }
        }

        mAlpha = alpha;

        if (!hasAlphaChannel())
        {
            // Set the alpha value this image is drawn at
            SDL_SetAlpha(mSDLSurface, SDL_SRCALPHA, (int) (255 * mAlpha));
        }
        else
        {
            if (SDL_MUSTLOCK(mSDLSurface))
                SDL_LockSurface(mSDLSurface);

            // Precompute as much as possible
            int maxHeight = std::min((mBounds.y + mBounds.h), mSDLSurface->h);
            int maxWidth = std::min((mBounds.x + mBounds.w), mSDLSurface->w);
            int i = 0;

            for (int y = mBounds.y; y < maxHeight; y++)
              for (int x = mBounds.x; x < maxWidth; x++)
              {
                  i = y * mSDLSurface->w + x;
                  // Only change the pixel if it was visible at load time...
                  Uint8 sourceAlpha = mAlphaChannel[i];
                  if (sourceAlpha > 0)
                  {
                      Uint8 r, g, b, a;
                      SDL_GetRGBA(((Uint32*) mSDLSurface->pixels)[i],
                                  mSDLSurface->format,
                                  &r, &g, &b, &a);

                      a = (Uint8) (sourceAlpha * mAlpha);

                      // Here is the pixel we want to set
                      ((Uint32 *)(mSDLSurface->pixels))[i] =
                      SDL_MapRGBA(mSDLSurface->format, r, g, b, a);
                  }
              }

            if (SDL_MUSTLOCK(mSDLSurface))
                SDL_UnlockSurface(mSDLSurface);
        }
    }
    else
    {
        mAlpha = alpha;
    }
}

Image *Image::SDLgetScaledImage(int width, int height)
{
    if (width == 0 || height == 0)
        return 0;

    // Increase our reference count and return ourselves in case of same size
    if (width == getWidth() && height == getHeight())
    {
        incRef();
        return this;
    }

    if (!mSDLSurface)
        return 0;

    ResourceManager *resman = ResourceManager::getInstance();

    // Generate a unique ID path for storing the scaled version in the
    // resource manager.
    std::string idPath = getIdPath();
    idPath += ":scaled:";
    idPath += toString(width);
    idPath += "x";
    idPath += toString(height);

    // Try whether a scaled version is already available
    Image *scaledImage = static_cast<Image*>(resman->get(idPath));

    if (!scaledImage)
    {
        // No scaled version with this size exists already, so create one
        SDL_Surface *scaledSurface = zoomSurface(mSDLSurface,
                                                 (double) width / getWidth(),
                                                 (double) height / getHeight(),
                                                 1);

        if (scaledSurface)
        {
            scaledImage = load(scaledSurface);
            SDL_FreeSurface(scaledSurface);

            // Place the scaled image in the resource manager
            resman->addResource(idPath, scaledImage);
        }
    }

    return scaledImage;
}

SDL_Surface* Image::SDLduplicateSurface(SDL_Surface* tmpImage)
{
    if (!tmpImage || !tmpImage->format)
        return NULL;

    return SDL_ConvertSurface(tmpImage, tmpImage->format, SDL_SWSURFACE);
}

Image *Image::_SDLload(SDL_Surface *tmpImage)
{
    if (!tmpImage)
        return NULL;

    bool hasAlpha = false;

    // The alpha channel to be filled with alpha values
    Uint8 *alphaChannel = new Uint8[tmpImage->w * tmpImage->h];

    if (tmpImage->format->BitsPerPixel == 32)
    {
        // Figure out whether the image uses its alpha layer
        for (int i = 0; i < tmpImage->w * tmpImage->h; ++i)
        {
            Uint8 r, g, b, a;
            SDL_GetRGBA(
                    ((Uint32*) tmpImage->pixels)[i],
                    tmpImage->format,
                    &r, &g, &b, &a);

            if (a != 255)
                hasAlpha = true;

            alphaChannel[i] = a;
        }
    }

    SDL_Surface *image;

    // Convert the surface to the current display format
    if (hasAlpha)
        image = SDL_DisplayFormatAlpha(tmpImage);
    else
    {
        image = SDL_DisplayFormat(tmpImage);

        // We also delete the alpha channel since
        // it's not used.
        delete[] alphaChannel;
        alphaChannel = NULL;
    }

    if (!image)
    {
        logger->log("Error: Image convert failed.");
        delete[] alphaChannel;
        return NULL;
    }

    return new Image(image, hasAlpha, alphaChannel);
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
            return NULL;
        }

        // Make sure the alpha channel is not used, but copied to destination
        SDL_SetAlpha(oldImage, 0, SDL_ALPHA_OPAQUE);
        SDL_BlitSurface(oldImage, NULL, image, NULL);
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

    return new SubImage(this, mSDLSurface,
                        mBounds.x + x,
                        mBounds.y + y,
                        width, height);
}

void Image::SDLterminateAlphaCache()
{
    SDLcleanCache();
    mUseAlphaCache = false;
}

//============================================================================
// SubImage Class
//============================================================================

SubImage::SubImage(Image *parent, SDL_Surface *image,
                   int x, int y, int width, int height):
    Image(image),
    mParent(parent)
{
    if (mParent)
    {
        mParent->incRef();
        mParent->SDLterminateAlphaCache();
        mHasAlphaChannel = mParent->hasAlphaChannel();
        mAlphaChannel = mParent->SDLgetAlphaChannel();
    }
    else
    {
        mHasAlphaChannel = false;
        mAlphaChannel = 0;
    }

    // Set up the rectangle.
    mBounds.x = x;
    mBounds.y = y;
    mBounds.w = width;
    mBounds.h = height;
    mUseAlphaCache = false;
}

#ifdef USE_OPENGL
SubImage::SubImage(Image *parent, GLuint image,
                   int x, int y, int width, int height,
                   int texWidth, int texHeight):
    Image(image, width, height, texWidth, texHeight),
    mParent(parent)
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
    mSDLSurface = 0;
    // Avoid possible destruction of its alpha channel
    mAlphaChannel = 0;
#ifdef USE_OPENGL
    mGLImage = 0;
#endif
    mParent->decRef();
}
