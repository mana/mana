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

#ifndef IMAGE_H
#define IMAGE_H

#include "resources/resource.h"

#include <SDL.h>

#ifdef USE_OPENGL

/* The definition of OpenGL extensions by SDL is giving problems with recent
 * gl.h headers, since they also include these definitions. As we're not using
 * extensions anyway it's safe to just disable the SDL version.
 */
#define NO_SDL_GLEXT

#include <SDL_opengl.h>
#endif

class Dye;
class Position;

/**
 * Defines a class for loading and storing images.
 */
class Image : public Resource
{
    friend class Graphics;
#ifdef USE_OPENGL
    friend class OpenGLGraphics;
#endif

    public:
        ~Image() override;

        /**
         * Loads an image from an SDL_RWops structure.
         *
         * @param rw         The SDL_RWops to load the image from.
         *
         * @return <code>NULL</code> if an error occurred, a valid pointer
         *         otherwise.
         */
        static Resource *load(SDL_RWops *rw);

        /**
         * Loads an image from an SDL_RWops structure and recolors it.
         *
         * @param rw         The SDL_RWops to load the image from.
         * @param dye        The dye used to recolor the image.
         *
         * @return <code>NULL</code> if an error occurred, a valid pointer
         *         otherwise.
         */
        static Resource *load(SDL_RWops *rw, Dye const &dye);

        /**
         * Loads an image from an SDL surface.
         */
        static Image *load(SDL_Surface *);

        /**
         * Frees the resources created by SDL.
         */
        void unload();

        /**
         * Tells is the image is loaded
         */
        bool isLoaded()
        { return mLoaded; }

        /**
         * Returns the width of the image.
         */
        int getWidth() const
        { return mBounds.w; }

        /**
         * Returns the height of the image.
         */
        int getHeight() const
        { return mBounds.h; }

        /**
         * Tells if the system is using OpenGL or SDL
         * @return true if OpenGL, false if SDL.
         */
        static bool useOpenGL();

        /**
         * Sets the alpha value of this image.
         */
        void setAlpha(float alpha);

        /**
         * Returns the alpha value of this image.
         */
        float getAlpha() const
        { return mAlpha; }

        /**
         * Creates a new image with the desired clipping rectangle.
         *
         * @return <code>NULL</code> if creation failed and a valid
         *         object otherwise.
         */
        Image *getSubImage(int x, int y, int width, int height);

        // SDL only public functions

        /**
         * Disable the transparency handling (for low CPUs in SDL Mode)
         */
        static void SDLdisableTransparency()
        { mDisableTransparency = true; }

        static bool SDLisTransparencyDisabled()
        { return mDisableTransparency; }

        static void setRenderer(SDL_Renderer *renderer);

#ifdef USE_OPENGL

        // OpenGL only public functions

        /**
         * Sets the target image format. Use <code>false</code> for SDL and
         * <code>true</code> for OpenGL.
         */
        static void setLoadAsOpenGL(bool useOpenGL);

        static bool getLoadAsOpenGL() { return mUseOpenGL; }

        int getTextureWidth() const { return mTexWidth; }
        int getTextureHeight() const { return mTexHeight; }
        static int getTextureType() { return mTextureType; }
#endif

    protected:

        // -----------------------
        // Generic protected members
        // -----------------------

        SDL_Rect mBounds;
        bool mLoaded;
        float mAlpha;

        // -----------------------
        // SDL protected members
        // -----------------------

        /** SDL Constructor */
        Image(SDL_Texture *texture, int width, int height);

        /** SDL_Surface to SDL_Texture Image loader */
        static Image *_SDLload(SDL_Surface *tmpImage);

        SDL_Texture *mTexture;

        /** Stores whether the transparency is disabled */
        static bool mDisableTransparency;

        static SDL_Renderer *mRenderer;

        // -----------------------
        // OpenGL protected members
        // -----------------------
#ifdef USE_OPENGL
        /**
         * OpenGL Constructor.
         */
        Image(GLuint glimage, int width, int height,
              int texWidth, int texHeight);

        /**
         * Returns the first power of two equal or bigger than the input.
         */
        static int powerOfTwo(int input);

        static Image *_GLload(SDL_Surface *image);

        GLuint mGLImage;
        int mTexWidth, mTexHeight;

        static bool mUseOpenGL;
        static bool mPowerOfTwoTextures;
        static int mTextureType;
        static int mTextureSize;
#endif
};

/**
 * A clipped version of a larger image.
 */
class SubImage : public Image
{
    public:
        SubImage(Image *parent, SDL_Texture *texture,
                 int x, int y, int width, int height);
#ifdef USE_OPENGL
        SubImage(Image *parent, GLuint image, int x, int y,
                 int width, int height, int texWidth, int textHeight);
#endif

        ~SubImage() override;

    private:
        Image *mParent;
};

#endif
