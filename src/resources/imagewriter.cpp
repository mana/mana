/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "resources/imagewriter.h"

#include "log.h"

#include <png.h>
#include <SDL.h>
#include <string>

bool ImageWriter::writePNG(SDL_Surface *surface, const std::string &filename)
{
    // TODO Maybe someone can make this look nice?
    FILE *fp = fopen(filename.c_str(), "wb");
    if (!fp)
    {
        logger->log("could not open file %s for writing", filename.c_str());
        return false;
    }

    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *row_pointers;
    int colortype;

    if (SDL_MUSTLOCK(surface)) {
        SDL_LockSurface(surface);
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!png_ptr)
    {
        logger->log("Had trouble creating png_structp");
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        logger->log("Could not create png_info");
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        logger->log("problem writing to %s", filename.c_str());
        return false;
    }

    png_init_io(png_ptr, fp);

    colortype = (surface->format->BitsPerPixel == 24) ?
        PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA;

    png_set_IHDR(png_ptr, info_ptr, surface->w, surface->h, 8, colortype,
            PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    png_set_packing(png_ptr);

    row_pointers = new png_bytep[surface->h];
    if (!row_pointers)
    {
        logger->log("Had trouble converting surface to row pointers");
        return false;
    }

    for (int i = 0; i < surface->h; i++)
    {
        row_pointers[i] = (png_bytep)(Uint8 *)surface->pixels + i * surface->pitch;
    }

    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);

    fclose(fp);

    delete [] row_pointers;

    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }

    return true;
}
