/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "dye.h"
#include "imagewriter.h"

using namespace std;

// return values
enum ReturnValues
{
    RETURN_OK = 0,
    INVALID_PARAMETER_LIST = 100,
    INVALID_INPUT_IMAGE = 101,
    INVALID_OUTPUT_IMAGE = 102,
    INVALID_DYE_PARAMETER = 105
};

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
    //SDL_FreeSurface(tmpImage); <-- We'll free the surface later.

    Uint32 *pixels = static_cast< Uint32 * >(surf->pixels);
    for (Uint32 *p_end = pixels + surf->w * surf->h; pixels != p_end; ++pixels)
    {
        int alpha = (*pixels >> rgba.Ashift) & 255;
        if (!alpha) continue;
        int v[3];

        v[0] = (*pixels >> rgba.Rshift) & 255;
        v[1] = (*pixels >> rgba.Gshift) & 255;
        v[2] = (*pixels >> rgba.Bshift) & 255;
        dye->update(v);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        *pixels = (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | alpha;
#else
        *pixels = v[0] | (v[1] << 8) | (v[2] << 16) | (alpha << 24);
#endif
    }

    return surf;
}

void printHelp()
{
    cout << endl
    << "This tool is used to dye item graphics used by the Mana client "
    << "according to the specification described here: "
    << endl << "http://doc.manasource.org/image_dyeing_system"
    << endl << endl <<
    "The tool expects 3 parameters:" << endl
    << "dyecmd <source_image> <target_image> <dye_string>" << endl
    << "e.g.:" << endl
    << "dyecmd \"armor-legs-shorts.png\" "
    <<"\"armor-legs-shorts2.png\" \"W:#222255,6666ff\"" << std::endl;
}

int main(int argc, char* argv[])
{
    Dye* dye = NULL;
    SDL_Surface* source = NULL, *target = NULL;
    ReturnValues returnValue = RETURN_OK;

    if (argc > 1 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")))
    {
        printHelp();
    }
    // not enough or to many parameters
    else if (argc != 4)
    {
        cout << INVALID_PARAMETER_LIST << " - INVALID_PARAMETER_LIST";
        printHelp();
        returnValue = INVALID_PARAMETER_LIST;
    }
    else
    {
        // Start dyeing process.
        string inputFile = argv[1];
        string outputFile = argv[2];
        string dyeDescription = argv[3];

        dye = new Dye(dyeDescription);
        if (!dye->loaded())
        {
            cout << INVALID_DYE_PARAMETER << " - INVALID_DYE_PARAMETER";
            printHelp();
            returnValue = INVALID_DYE_PARAMETER;
        }
        else
        {
            source = IMG_Load(inputFile.c_str());
            if (!source)
            {
                cout << INVALID_INPUT_IMAGE << " - INVALID_INPUT_IMAGE";
                printHelp();
                returnValue = INVALID_INPUT_IMAGE;
            }
            else
            {
                target = recolor(source, dye);

                if (!ImageWriter::writePNG(target, outputFile))
                {
                    cout << INVALID_OUTPUT_IMAGE << " - INVALID_OUTPUT_IMAGE";
                    printHelp();
                    returnValue = INVALID_OUTPUT_IMAGE;
                }
            } // Valid source image file
        } // Valid dye parameter
    } // Parameters ok

    if (source)
        SDL_FreeSurface(source);
    if (target)
        SDL_FreeSurface(target);
    if (dye)
        delete dye;

    return returnValue;
}
