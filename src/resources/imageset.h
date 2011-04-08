/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#ifndef IMAGESET_H
#define IMAGESET_H

#include "resources/resource.h"

#include <vector>

class Image;

/**
 * Stores a set of subimages originating from a single image.
 */
class ImageSet : public Resource
{
    public:
        /**
         * Cuts the passed image in a grid of sub images.
         */
        ImageSet(Image *img, int w, int h, int margin = 0, int spacing = 0);

        ~ImageSet();

        /**
         * Returns the width of the images in the image set.
         */
        int getWidth() const { return mWidth; }

        /**
         * Returns the height of the images in the image set.
         */
        int getHeight() const { return mHeight; }

        typedef std::vector<Image*>::size_type size_type;
        Image* get(size_type i) const;

        size_type size() const { return mImages.size(); }

    private:
        std::vector<Image*> mImages;

        int mHeight; /**< Height of the images in the image set. */
        int mWidth;  /**< Width of the images in the image set. */
};

#endif
