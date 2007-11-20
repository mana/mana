/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
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
 *  $Id: imageloader.cpp 3515 2007-08-25 16:56:52Z gmelquio $
 */

#include <cassert>
#include <string>
#include <guichan/color.hpp>
#include <guichan/sdl/sdlpixel.hpp>

#include "imageloader.h"

#include "image.h"
#include "resourcemanager.h"

ProxyImage::ProxyImage(SDL_Surface *s):
    mImage(NULL), mSDLImage(s)
{
}

ProxyImage::~ProxyImage()
{
    free();
}

void ProxyImage::free()
{
    if (mSDLImage)
    {
        SDL_FreeSurface(mSDLImage);
        mSDLImage = NULL;
    }
    else if (mImage)
    {
        delete mImage;
        mImage = NULL;
    }
}

int ProxyImage::getWidth() const
{
    return mSDLImage ? mSDLImage->w : mImage->getWidth();
}

int ProxyImage::getHeight() const
{
    return mSDLImage ? mSDLImage->h : mImage->getHeight();
}

gcn::Color ProxyImage::getPixel(int x, int y)
{
    assert(mSDLImage);
    return gcn::SDLgetPixel(mSDLImage, x, y);
}

void ProxyImage::putPixel(int x, int y, gcn::Color const &color)
{
    assert(mSDLImage);
    gcn::SDLputPixel(mSDLImage, x, y, color);
}

void ProxyImage::convertToDisplayFormat()
{
    assert(mSDLImage);
    /* The picture is most probably full of the pink pixels Guichan uses for
       transparency, as this function will only be called when setting an image
       font. Get rid of them. */
    SDL_SetColorKey(mSDLImage, SDL_SRCCOLORKEY,
                    SDL_MapRGB(mSDLImage->format, 255, 0, 255));
    mImage = ::Image::load(mSDLImage);
    SDL_FreeSurface(mSDLImage);
    mSDLImage = NULL;
}

gcn::Image *ImageLoader::load(std::string const &filename, bool convert)
{
    ResourceManager *resman = ResourceManager::getInstance();
    ProxyImage *i = new ProxyImage(resman->loadSDLSurface(filename));
    if (convert) i->convertToDisplayFormat();
    return i;
}
