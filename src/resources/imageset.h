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
        ImageSet(Image *img, int w, int h);

        /**
         * Destructor.
         */
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
