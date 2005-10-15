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

#ifndef _TMW_IMAGE_H
#define _TMW_IMAGE_H

#include <SDL.h>
#ifdef USE_OPENGL
#include <SDL_opengl.h>
#endif

#include "resource.h"

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
        /**
         * Destructor.
         */
        virtual
        ~Image();

        /**
         * Loads an image from a buffer in memory.
         *
         * @param buffer     The memory buffer containing the image data.
         * @param bufferSize The size of the memory buffer in bytes.
         *
         * @return <code>NULL</code> if the an error occurred, a valid pointer
         *         otherwise.
         */
        static Image*
        load(void* buffer, unsigned int bufferSize, const std::string &idPath);

        /**
         * Frees the resources created by SDL.
         */
        virtual void
        unload();

        /**
         * Returns the width of the image.
         */
        virtual int
        getWidth() const { return bounds.w; }


        /**
         * Returns the height of the image.
         */
        virtual int
        getHeight() const { return bounds.h; }

        /**
         * Creates a new image with the desired clipping rectangle.
         *
         * @return <code>NULL</code> if creation failed and a valid
         *         object otherwise.
         */
        virtual Image*
        getSubImage(int x, int y, int width, int height);

        /**
         * Sets the alpha value of this image.
         */
        void
        setAlpha(float alpha);

        /**
         * Returns the alpha value of this image.
         */
        float
        getAlpha();

#ifdef USE_OPENGL
        /**
         * Sets the target image format. Use <code>false</code> for SDL and
         * <code>true</code> for OpenGL.
         */
        static void setLoadAsOpenGL(bool useOpenGL);
#endif


    protected:
        /**
         * Constructor.
         */
#ifdef USE_OPENGL
        Image(const std::string &idPath, GLuint glimage, int width, int height,
                int texWidth, int texHeight);
#endif
        Image(const std::string &idPath, SDL_Surface *image);

        SDL_Rect bounds;
        bool loaded;

#ifdef USE_OPENGL
        GLuint mGLImage;
        int mTexWidth, mTexHeight;

        static bool mUseOpenGL;
#endif
        SDL_Surface *mImage;
        float mAlpha;
};

/**
 * A clipped version of a larger image.
 */
class SubImage : public Image
{
    public:
        /**
         * Constructor.
         */
        SubImage(Image *parent, SDL_Surface *image,
                 int x, int y, int width, int height);
#ifdef USE_OPENGL
        SubImage(Image *parent, GLuint image, int x, int y,
                 int width, int height, int texWidth, int textHeight);
#endif

        /**
         * Destructor.
         */
        ~SubImage();

        /**
         * Creates a new image with the desired clipping rectangle.
         *
         * @return <code>NULL</code> if creation failed and a valid
         *         image otherwise.
         */
        Image*
        getSubImage(int x, int y, int width, int height);

    private:
        Image *mParent;
};

#endif
