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

#pragma once

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

        ImageSet(const ImageSet&) = delete;
        ImageSet& operator=(const ImageSet&) = delete;

        ~ImageSet() override;

        /**
         * Returns the width of the images in the image set.
         */
        int getWidth() const { return mWidth; }

        /**
         * Returns the height of the images in the image set.
         */
        int getHeight() const { return mHeight; }

        Image *get(size_t i) const;

        size_t size() const { return mImages.size(); }

        int getOffsetX() const
        { return mOffsetX; }

        void setOffsetX(int n)
        { mOffsetX = n; }

        int getOffsetY() const
        { return mOffsetY; }

        void setOffsetY(int n)
        { mOffsetY = n; }

    private:
        std::vector<Image*> mImages;

        int mWidth;  /**< Width of the images in the image set. */
        int mHeight; /**< Height of the images in the image set. */
        int mOffsetX = 0;
        int mOffsetY = 0;
};
