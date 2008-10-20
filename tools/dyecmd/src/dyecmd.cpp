/*
 *  The Mana World
 *  Copyright 2008 The Mana World Development Team
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

#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>



#include "dye.h"
#include "imagewriter.h"

using namespace std;

// return values
#define RETURN_OK                 1
#define INVALID_PARAMETER_LIST  100
#define INVALID_INPUT_IMAGE     101
#define INVALID_OUTPUT_IMAGE    102
#define INVALID_DYE_PARAMETER   105

SDL_Surface* recolor(SDL_Surface* tmpImage, Dye* dye)
{
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
    //SDL_FreeSurface(tmpImage);

    Uint32 *pixels = static_cast< Uint32 * >(surf->pixels);
    for (Uint32 *p_end = pixels + surf->w * surf->h; pixels != p_end; ++pixels)
    {
        int alpha = *pixels & 255;
        if (!alpha) continue;
        int v[3];
        v[0] = (*pixels >> 24) & 255;
        v[1] = (*pixels >> 16) & 255;
        v[2] = (*pixels >> 8 ) & 255;
        dye->update(v);
        *pixels = (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | alpha;
    }

    return surf;
}

int main(int argc, char* argv[])
{
    Dye* dye = NULL;
    SDL_Surface* source = NULL;

    // not enough or to many parameters
    if (argc != 4)
    {
        cout << INVALID_PARAMETER_LIST << " - INVALID_PARAMETER_LIST";
        exit(INVALID_PARAMETER_LIST);
    }

    try
    {
        dye = new Dye(argv[3]);
    }
    catch (exception &e)
    {
        cout << INVALID_DYE_PARAMETER << " - INVALID_DYE_PARAMETER";
        exit(INVALID_DYE_PARAMETER);
    }

    try
    {
        source = IMG_Load(argv[1]);
        if (!source)
        {
            throw;
        }
    }
    catch (exception &e)
    {
        cout << INVALID_INPUT_IMAGE << " - INVALID_INPUT_IMAGE";
        exit(INVALID_INPUT_IMAGE);
    }

    SDL_Surface* target = recolor(source, dye);

    if (!ImageWriter::writePNG(target, argv[2]))
    {
        cout << INVALID_OUTPUT_IMAGE << " - INVALID_OUTPUT_IMAGE";
        exit(INVALID_OUTPUT_IMAGE);
    }

    SDL_FreeSurface(source);
    SDL_FreeSurface(target);
    delete dye;

    return 0;
}
