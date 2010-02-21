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

#ifndef RESOURCES_AMBIENTOVERLAY_H
#define RESOURCES_AMBIENTOVERLAY_H

class Graphics;
class Image;

class AmbientLayer
{
    public:
        /**
         * Constructor.
         *
         * @param img       the image this overlay displays
         * @param parallax  scroll factor based on camera position
         * @param speedX    scrolling speed in x-direction
         * @param speedY    scrolling speed in y-direction
         * @param keepRatio  rescale the image to keep
         * the same ratio than in 800x600 resolution mode.
         */
        AmbientLayer(Image *img, float parallax,
                       float speedX, float speedY, bool keepRatio = false);

        ~AmbientLayer();

        void update(int timePassed, float dx, float dy);

        void draw(Graphics *graphics, int x, int y);

    private:
        Image *mImage;
        float mParallax;
        float mPosX;              /**< Current layer X position. */
        float mPosY;              /**< Current layer Y position. */
        float mSpeedX;            /**< Scrolling speed in X direction. */
        float mSpeedY;            /**< Scrolling speed in Y direction. */
        bool mKeepRatio;          /**< Keep overlay ratio on every resolution */
};

#endif
