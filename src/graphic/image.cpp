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
 *  By ElvenProgrammer aka Eugenio Favalli (umperio@users.sourceforge.net)
 */

#include "image.h"


// VideoImage

VideoImage::VideoImage(BITMAP *src, int offset_x, int offset_y):
    src(src),
    offset_x(offset_x),
    offset_y(offset_y)
{
}

VideoImage::~VideoImage() {
    destroy_bitmap(src);
}

void VideoImage::draw(BITMAP *dst, int x, int y) {
    //SDL_Rect dst_rect;
    //dst_rect.x = x + offset_x;
    //dst_rect.y = y + offset_y;
    //SDL_BlitSurface(src, NULL, dst, &dst_rect);

    masked_blit(src, dst, 0, 0, x + offset_x, y + offset_y, src->w, src->h);
}


// Spriteset

Spriteset::Spriteset(BITMAP *bmp, int width, int height, int offx, int offy)
{
    /*
     * We're creating sub bitmaps here for plain convenience. With SDL this'll
     * probably need to be done different.
     */
    int x, y;

    for (y = 0; y + height <= bmp->h; y += height)
    {
        for (x = 0; x + width <= bmp->w; x += width)
        {
            spriteset.push_back(new VideoImage(
                        create_sub_bitmap(bmp, x, y, width, height),
                        offx, offy));
        }
    }
}

Spriteset::~Spriteset()
{
    for (unsigned int i = 0; i < spriteset.size(); i++) {
        delete spriteset[i];
    }
}

int Spriteset::getProperty(DATAFILE *datafile, int type) {
    return atoi(get_datafile_property(datafile, type));
}  
