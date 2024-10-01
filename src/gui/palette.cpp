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

#include <cmath>

static constexpr double PI = 3.14159265;
const gcn::Color Palette::BLACK = gcn::Color(0, 0, 0);
Timer Palette::mRainbowTimer;
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
    mColors(size)
{
    mInstances.insert(this);
}

Palette::~Palette()
{
    mInstances.erase(this);
}

const gcn::Color &Palette::getColor(char c, bool &valid)
 {
    for (const auto &color : mColors)
    {
        if (color.ch == c)
        {
            valid = true;
            return color.color;
        }
    }
    valid = false;
    return BLACK;
}

void Palette::advanceGradients()
{
    const int advance = mRainbowTimer.elapsed() / 5;
    if (advance <= 0)
        return;

    mRainbowTimer.extend(advance * 5);

    for (auto palette : mInstances)
        palette->advanceGradient(advance);
}

void Palette::advanceGradient(int advance)
{
    for (auto elem : mGradVector)
    {
        int delay = elem->delay;

        if (elem->grad == PULSE)
            delay = delay / 20;

        const int numOfColors = (elem->grad == SPECTRUM ? 6 :
                                 elem->grad == PULSE ? 127 :
                                 RAINBOW_COLOR_COUNT);

        elem->gradientIndex = (elem->gradientIndex + advance) %
                              (delay * numOfColors);

        const int pos = elem->gradientIndex % delay;
        const int colIndex = elem->gradientIndex / delay;

        if (elem->grad == PULSE)
        {
            const int colVal = (int) (255.0 * sin(PI * colIndex / numOfColors));
            const gcn::Color &col = elem->testColor;

            elem->color.r = ((colVal * col.r) / 255) % (col.r + 1);
            elem->color.g = ((colVal * col.g) / 255) % (col.g + 1);
            elem->color.b = ((colVal * col.b) / 255) % (col.b + 1);
        }
        if (elem->grad == SPECTRUM)
        {
            int colVal;

            if (colIndex % 2)
            { // falling curve
                colVal = (int)(255.0 * (cos(PI * pos / delay) + 1) / 2);
            }
            else
            { // ascending curve
                colVal = (int)(255.0 * (cos(PI * (delay - pos) / delay) +
                                1) / 2);
            }

            elem->color.r =
                    (colIndex == 0 || colIndex == 5) ? 255 :
                    (colIndex == 1 || colIndex == 4) ? colVal : 0;
            elem->color.g =
                    (colIndex == 1 || colIndex == 2) ? 255 :
                    (colIndex == 0 || colIndex == 3) ? colVal : 0;
            elem->color.b =
                    (colIndex == 3 || colIndex == 4) ? 255 :
                    (colIndex == 2 || colIndex == 5) ? colVal : 0;
        }
        else if (elem->grad == RAINBOW)
        {
            const gcn::Color &startCol = RAINBOW_COLORS[colIndex];
            const gcn::Color &destCol =
                    RAINBOW_COLORS[(colIndex + 1) % numOfColors];

            const double startColVal = (cos(PI * pos / delay) + 1) / 2;
            const double destColVal = 1 - startColVal;

            elem->color.r =(int)(startColVal * startCol.r +
                                            destColVal * destCol.r);

            elem->color.g =(int)(startColVal * startCol.g +
                                            destColVal * destCol.g);

            elem->color.b =(int)(startColVal * startCol.b +
                                            destColVal * destCol.b);
        }
    }
}
