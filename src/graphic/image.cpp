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


// Image

Image::Image(int offset_x, int offset_y) {
    this->offset_x = offset_x;
    this->offset_y = offset_y;
}


// RleImage

RleImage::RleImage(RLE_SPRITE *src, int offset_x, int offset_y):
    Image(offset_x, offset_y)
{
    this->src = src;
}

RleImage::~RleImage() {
    destroy_rle_sprite(src);
}

void RleImage::draw(BITMAP *dest, int x, int y) {
    draw_rle_sprite(dest, src, x + offset_x, y + offset_y);
}    


// VideoImage

VideoImage::VideoImage(BITMAP *src, int offset_x, int offset_y):
    Image(offset_x, offset_y)
{
    this->src = src;
}

VideoImage::~VideoImage() {
    destroy_bitmap(src);
}

void VideoImage::draw(BITMAP *dest, int x, int y) {
    masked_blit(src, dest, 0, 0, x + offset_x, y + offset_y, src->w, src->h);
}


// Spriteset

Spriteset::Spriteset(std::string filename)
{
    DATAFILE *datafile = load_datafile(filename.c_str());
    if (!datafile)error("Unable to load graphic file: " + filename);
    int i = 0;
    while (datafile[i].type != DAT_END) {
        Image *temp_image;
        if (gfx_capabilities & GFX_HW_VRAM_BLIT) {
            BITMAP *temp_video_bitmap = create_video_bitmap(
                    ((RLE_SPRITE *)datafile[i].dat)->w,
                    ((RLE_SPRITE *)datafile[i].dat)->h);
            if (temp_video_bitmap) {
                clear_to_color(temp_video_bitmap, makecol(255, 0, 255));
                draw_rle_sprite(temp_video_bitmap, 
                        (RLE_SPRITE *)datafile[i].dat, 0, 0);
                temp_image = new VideoImage(temp_video_bitmap,
                        getProperty(&datafile[i], DAT_ID('X','C','R','P')),
                        getProperty(&datafile[i], DAT_ID('Y','C','R','P')));
            } else {
                warning("You ran out of video memory!");
                temp_image = new RleImage(
                        (RLE_SPRITE*)datafile[i].dat,
                        getProperty(&datafile[i], DAT_ID('X','C','R','P')),
                        getProperty(&datafile[i], DAT_ID('Y','C','R','P')));
            }
        } else {
            temp_image = new RleImage(
                    (RLE_SPRITE*)datafile[i].dat,
                    getProperty(&datafile[i], DAT_ID('X','C','R','P')),
                    getProperty(&datafile[i], DAT_ID('Y','C','R','P')));
        }
        spriteset.push_back(temp_image);
        i++;
    }
}

Spriteset::Spriteset(BITMAP *bmp, int width, int height)
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
                        30, 40));
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
