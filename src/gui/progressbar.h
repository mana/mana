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

#ifndef __PROGRESSBAR_H__
#define __PROGRESSBAR_H__

#include <allegro.h>
#include <guichan.hpp>

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
        ProgressBar(float progress = 0.0f, int x = 0, int y = 0, int width = 40, unsigned char red = 150, unsigned green = 150, unsigned char blue = 150);

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
	* Change the filling of the progress bar...
	*/
	void setColor(unsigned char MyRed, unsigned char MyGreen, unsigned char MyBlue)
	{
		Red = MyRed; Green = MyGreen; Blue = MyBlue;
	}; 
	
	int getHeight()
	{ return 7; };
	
	int getWidth()
	{ return Width; };
	
	int getX()
	{ return X; };
	
	int getY()
	{ return Y; };

    private:
        float progress;
	unsigned char Red, Green, Blue;
	int X, Y, Width;
};

#endif
