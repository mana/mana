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

#include "sdlimageloader.h"

#include <iostream>
#include <string>
#include <SDL_image.h>

#include <guichan/exception.hpp>

#include "resourcemanager.h"

void SDLImageLoader::prepare(const std::string &filename)
{
    if (mCurrentImage)
    {
        throw GCN_EXCEPTION("Function called before finalizing or discarding last loaded image.");
    }

    ResourceManager *resman = ResourceManager::getInstance();

    int fileSize;
    void *buffer = resman->loadFile(filename, fileSize);

    SDL_Surface *tmp = NULL;
    if (buffer) {
        SDL_RWops *rw = SDL_RWFromMem(buffer, fileSize);
        tmp = IMG_Load_RW(rw, 1);
        ::free(buffer);
    }

    if (!tmp)
    {
        throw GCN_EXCEPTION(std::string("Unable to load image file: ")+filename);
    }

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

    mCurrentImage = SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, 32,
            rmask, gmask, bmask, amask);

    if (!mCurrentImage)
    {
        throw GCN_EXCEPTION(std::string("Not enough memory to load: ")+filename);
    }

    SDL_Surface* tmp2 = SDL_ConvertSurface(tmp, mCurrentImage->format, SDL_SWSURFACE);
    SDL_FreeSurface(tmp);
    SDL_FreeSurface(mCurrentImage);

    mCurrentImage = tmp2;
}
