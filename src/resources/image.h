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

#include "resource.h"
//#include <SDL/SDL.h>
#include <allegro.h>

/**
 * A clipped version of a larger image.
 */
class SubImage;

/**
 * Defines a class for loading and storing images.
 */
class Image : public Resource
{
    public:
        /**
         * Constructor.
         */
        Image(BITMAP *image);

        /**
         * Destructor.
         */
        virtual ~Image();

        /**
         * Loads an image.
         * @param filePath The path to the image file to load.
         * @return <code>true</code> if the image was loaded
         * <code>false</code> otherwise.
         */
        static Image *load(const std::string &filePath);

        /**
         * Frees the resources created by SDL.
         */
        void unload();

        /**
         * Creates a new image with the desired clipping rectangle.
         * @return <code>NULL</code> if creation failed and a valid
         * object otherwise.
         */
        Image* createSubImage(int x, int y, int width, int height);

        /**
         * Attempts to blit the internal image onto the screen.
         * @return <code>true</code> if the image was blitted properly
         * <code>false</code> otherwise.
         */
        bool draw(BITMAP *screen, int x, int y);

    protected:
        //SDL_Rect screenRect;
        //SDL_Surface *image;
        BITMAP *image;
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
        //SubImage(SDL_Surface *timage, int x, int y, int width, int height);
        SubImage(Image *parent, BITMAP *image,
                int x, int y, int width, int height);

        /**
         * Destructor.
         */
        ~SubImage();

        /**
         * Redefines unload to not do anything.
         */
        void unload();

        /**
         * Draws the clipped image onto the screen.
         * @return <code>true</code> if drawing was succesful
         * <code>false</code> otherwise.
         */
        bool draw(BITMAP *screen, int x, int y);

    private:
        Image *parent;
        //BITMAP *image;
        //SDL_Rect clipRect;
        //SDL_Rect screenRect;
        //SDL_Surface *image;
        //SDL_Surface *screen;
        //unsigned int referenceCount;
};

#endif
