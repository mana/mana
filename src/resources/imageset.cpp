/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "resources/imageset.h"

#include "log.h"

#include "resources/image.h"

#include "utils/dtor.h"

ImageSet::ImageSet(Image *img, int width, int height,
                   int margin, int spacing) :
    mWidth(width),
    mHeight(height)
{
    for (int y = margin; y + height <= img->getHeight() - margin; y += height + spacing)
    {
        for (int x = margin; x + width <= img->getWidth() - margin; x += width + spacing)
        {
            mImages.push_back(img->getSubImage(x, y, width, height));
        }
    }
}

ImageSet::~ImageSet()
{
    delete_all(mImages);
}

Image *ImageSet::get(size_t i) const
{
    if (i >= mImages.size())
    {
        Log::warn("No sprite %d in this image set", (int) i);
        return nullptr;
    }

    return mImages[i];
}
