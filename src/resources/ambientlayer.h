/*
 *  The Mana Client
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

#include "resource.h"

class Graphics;
class Image;

class AmbientLayer
{
    public:
        /**
         * Constructor.
         *
         * @param img       the image this overlay displays
         */
        AmbientLayer(Image *img);
        ~AmbientLayer();

        void update(int timePassed, float dx, float dy);

        void draw(Graphics *graphics);

        float mParallax = 0;      /**< Scroll factor based on camera position. */
        float mSpeedX = 0;        /**< Scrolling speed in X direction. */
        float mSpeedY = 0;        /**< Scrolling speed in Y direction. */
        int mMask = 1;
        bool mKeepRatio = false;  /**< Keep overlay ratio on every resolution like in 800x600 */

    private:
        ResourceRef<Image> mImage;
        float mPosX = 0;          /**< Current layer X position. */
        float mPosY = 0;          /**< Current layer Y position. */
};
