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
 *  $Id: imageset.cpp 4209 2008-04-29 12:58:21Z b_lindeijer $
 */

#include <algorithm>

#include "imageset.h"

#include "../log.h"

#include "image.h"

#include "../utils/dtor.h"

ImageSet::ImageSet(Image *img, int width, int height)
{
    for (int y = 0; y + height <= img->getHeight(); y += height)
    {
        for (int x = 0; x + width <= img->getWidth(); x += width)
        {
            mImages.push_back(img->getSubImage(x, y, width, height));
        }
    }
    mWidth = width;
    mHeight = height;
}

ImageSet::~ImageSet()
{
    for_each(mImages.begin(), mImages.end(), make_dtor(mImages));
}

Image*
ImageSet::get(size_type i)
{
    if (i >= mImages.size())
    {
        logger->log("Warning: No sprite %d in this image set", (int) i);
        return NULL;
    }
    else
    {
        return mImages[i];
    }
}
