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

#include "../log.h"
#include "image.h"
#include <iostream>

Image::Image(BITMAP *image):
    image(image)
{
}

Image::~Image()
{
    unload();
}

Image* Image::load(const std::string &filePath)
{
    // Attempt to use SDL_Image to load the file.
    //image = IMG_Load(filePath.c_str());
    std::cout << "Attempting to load image from " << filePath << std::endl;
    BITMAP *image = load_bitmap(filePath.c_str(), NULL);

    // Check if the file was opened and return the appropriate value.
    if (!image) {
        //log("Error", "Image load failed : %s", IMG_GetError());
        log("Error", "Image load failed : %s", filePath.c_str());
        return NULL;
    }

    return new Image(image);
}

void Image::unload()
{
    // Free the image surface.
    if (image != NULL) {
        //SDL_FreeSurface(image);
        destroy_bitmap(image);
        image = NULL;
        loaded = false;
    }
}


int Image::getWidth() const
{
    if (image != NULL) {
        return image->w;
    }
    return 0;
}

int Image::getHeight() const
{
    if (image != NULL) {
        return image->h;
    }
    return 0;
}

Image* Image::getSubImage(int x, int y, int width, int height)
{
    // Create a new clipped sub-image
    return new SubImage(this, image, x, y, width, height);
}

Image* Image::getScaledInstance(int width, int height)
{
    return new ScaledImage(this, image, width, height);
}

bool Image::draw(BITMAP *screen, int x, int y)
{
    // Check that preconditions for blitting are met.
    if (screen == NULL || image == NULL) return false;

    //SDL_Rect dst_rect;
    //dst_rect.x = x + offset_x;
    //dst_rect.y = y + offset_y;
    //SDL_BlitSurface(src, NULL, dst, &dst_rect);
    // Draw the image onto the screen.
    draw_sprite(screen, image, x, y);
    //if (SDL_BlitSurface(image, NULL, screen, &screenRect) < 0) {
    //    return false;
    //}

    return true;
}

void Image::drawPattern(BITMAP *screen, int x, int y, int w, int h)
{
    int iw = getWidth();              // Width of image
    int ih = getHeight();             // Height of image
    if (iw == 0 || ih == 0) return;

    int px = 0;                       // X position on pattern plane
    int py = 0;                       // Y position on pattern plane

    while (py < h) {
        while (px < w) {
            draw(screen, x + px, y + py);
            // TODO: Prevent overdraw
            px += iw;
        }
        py += ih;
        px = x;
    }
}

//============================================================================

SubImage::SubImage(Image *parent, BITMAP *image,
        int x, int y, int width, int height):
    Image(create_sub_bitmap(image, x, y, width, height)),
    parent(parent)
{
    //this->image = create_sub_bitmap(image, x, y, width, height);
    parent->incRef();
    // Set up the clipping rectangle.
    //clipRect.x = x;
    //clipRect.y = y;
    //clipRect.w = width;
    //clipRect.h = height;
}

SubImage::~SubImage()
{
    if (image) {
        destroy_bitmap(image);
        image = NULL;
    }
    // TODO: Enable when no longer a problem
    //parent->decRef();
}

bool SubImage::draw(BITMAP *screen, int x, int y)
{
    // Check that drawing preconditions are satisfied.
    if (screen == NULL || image == NULL) return false;

    // Draw the image onto the screen.
    draw_sprite(screen, image, x, y);
    //if (SDL_BlitSurface(image, &clipRect, screen, &screenRect) < 0) {
    //    return false;
    //}

    return true;
}

//============================================================================

ScaledImage::ScaledImage(Image *parent, BITMAP *bmp, int width, int height):
    Image(create_bitmap(width, height))
{
    if (image) {
        stretch_blit(bmp, image, 0, 0, bmp->w, bmp->h, 0, 0, width, height);
    }
}

ScaledImage::~ScaledImage()
{
    if (image) {
        destroy_bitmap(image);
        image = NULL;
    }
}
