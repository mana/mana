/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "palette.h"

#include "configuration.h"
#include "client.h"

#include "gui/gui.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <math.h>

static const double PI = 3.14159265;
const gcn::Color Palette::BLACK = gcn::Color(0, 0, 0);
Palette::Palettes Palette::mInstances;

const gcn::Color Palette::RAINBOW_COLORS[7] = {
    gcn::Color(255, 0, 0),
    gcn::Color(255, 153, 0),
    gcn::Color(255, 255, 0),
    gcn::Color(0, 153, 0),
    gcn::Color(0, 204, 204),
    gcn::Color(51, 0, 153),
    gcn::Color(153, 0, 153)
};
/** Number of Elemets of RAINBOW_COLORS */
const int Palette::RAINBOW_COLOR_COUNT = 7;

Palette::Palette(int size) :
    mRainbowTime(tick_time),
    mColors(Colors(size))
{
    mInstances.insert(this);
}

Palette::~Palette()
{
    mInstances.erase(this);
}

const gcn::Color& Palette::getColor(char c, bool &valid)
 {
    for (Colors::const_iterator col = mColors.begin(),
         colEnd = mColors.end(); col != colEnd; ++col)
    {
        if (col->ch == c)
        {
            valid = true;
            return col->color;
        }
    }
    valid = false;
    return BLACK;
}

void Palette::advanceGradients()
{
    auto it = mInstances.begin();
    auto it_end = mInstances.end();

    for (; it != it_end; it++)
    {
        (*it)->advanceGradient();
    }
}

void Palette::advanceGradient()
{
    if (get_elapsed_time(mRainbowTime) > 5)
    {
        int pos, colIndex, colVal, delay, numOfColors;
        // For slower systems, advance can be greater than one (advance > 1
        // skips advance-1 steps). Should make gradient look the same
        // independent of the framerate.
        int advance = get_elapsed_time(mRainbowTime) / 5;
        double startColVal, destColVal;

        for (size_t i = 0; i < mGradVector.size(); i++)
        {
            delay = mGradVector[i]->delay;

            if (mGradVector[i]->grad == PULSE)
                delay = delay / 20;

            numOfColors = (mGradVector[i]->grad == SPECTRUM ? 6 :
                           mGradVector[i]->grad == PULSE ? 127 :
                           RAINBOW_COLOR_COUNT);

            mGradVector[i]->gradientIndex =
                                    (mGradVector[i]->gradientIndex + advance) %
                                    (delay * numOfColors);

            pos = mGradVector[i]->gradientIndex % delay;
            colIndex = mGradVector[i]->gradientIndex / delay;

            if (mGradVector[i]->grad == PULSE)
            {
                colVal = (int) (255.0 * sin(PI * colIndex / numOfColors));

                const gcn::Color &col = mGradVector[i]->testColor;

                mGradVector[i]->color.r = ((colVal * col.r) / 255) % (col.r + 1);
                mGradVector[i]->color.g = ((colVal * col.g) / 255) % (col.g + 1);
                mGradVector[i]->color.b = ((colVal * col.b) / 255) % (col.b + 1);
            }
            if (mGradVector[i]->grad == SPECTRUM)
            {
                if (colIndex % 2)
                { // falling curve
                    colVal = (int)(255.0 * (cos(PI * pos / delay) + 1) / 2);
                }
                else
                { // ascending curve
                    colVal = (int)(255.0 * (cos(PI * (delay - pos) / delay) +
                                   1) / 2);
                }

                mGradVector[i]->color.r =
                        (colIndex == 0 || colIndex == 5) ? 255 :
                        (colIndex == 1 || colIndex == 4) ? colVal : 0;
                mGradVector[i]->color.g =
                        (colIndex == 1 || colIndex == 2) ? 255 :
                        (colIndex == 0 || colIndex == 3) ? colVal : 0;
                mGradVector[i]->color.b =
                        (colIndex == 3 || colIndex == 4) ? 255 :
                        (colIndex == 2 || colIndex == 5) ? colVal : 0;
            }
            else if (mGradVector[i]->grad == RAINBOW)
            {
                const gcn::Color &startCol = RAINBOW_COLORS[colIndex];
                const gcn::Color &destCol =
                        RAINBOW_COLORS[(colIndex + 1) % numOfColors];

                startColVal = (cos(PI * pos / delay) + 1) / 2;
                destColVal = 1 - startColVal;

                mGradVector[i]->color.r =(int)(startColVal * startCol.r +
                                               destColVal * destCol.r);

                mGradVector[i]->color.g =(int)(startColVal * startCol.g +
                                               destColVal * destCol.g);

                mGradVector[i]->color.b =(int)(startColVal * startCol.b +
                                               destColVal * destCol.b);
            }
        }

        if (advance)
            mRainbowTime = tick_time;
    }
}
