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

#ifndef _TMW_PROGRESSBAR_H
#define _TMW_PROGRESSBAR_H

#include <guichan.hpp>
#include "../resources/image.h"

/**
 * A progress bar.
 *
 * \ingroup GUI
 */
class ProgressBar : public gcn::Widget {
    public:
        /**
         * Constructor, initializes the progress with the given value.
         */
        ProgressBar(float progress = 0.0f, int x = 0, int y = 0,
                int width = 40, int height = 7,
                int red = 150, int green = 150, int blue = 150);

        /**
         * Destructor.
         */
        ~ProgressBar();

        /**
         * Draws the progress bar.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Sets the current progress.
         */
        void setProgress(float progress);

        /**
         * Returns the current progress.
         */
        float getProgress();

        /**
         * Change the filling of the progress bar.
         */
        void setColor(int red, int green, int blue);

        /**
         * Get The red value of color
         */
        unsigned char getRed()
        {
            return red;
        }

         /**
         * Get The red value of color
         */
        unsigned char getGreen()
        {
            return green;
        }

         /**
         * Get The red value of color
         */
        unsigned char getBlue()
        {
            return blue;
        }

    private:
        float progress;
        int red, green, blue;

        // Bar Images
        Image *dBackground;
        Image *dTopLeftBorder, *dTopRightBorder, *dBottomLeftBorder;
        Image *dBottomRightBorder;
        Image *dLeftBorder, *dRightBorder, *dTopBorder, *dBottomBorder;
};

#endif
