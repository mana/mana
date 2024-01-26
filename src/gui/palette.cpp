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

#include <cmath>

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

        for (auto &elem : mGradVector)
        {
            delay = elem->delay;

            if (elem->grad == PULSE)
                delay = delay / 20;

            numOfColors = (elem->grad == SPECTRUM ? 6 :
                           elem->grad == PULSE ? 127 :
                           RAINBOW_COLOR_COUNT);

            elem->gradientIndex =
                                    (elem->gradientIndex + advance) %
                                    (delay * numOfColors);

            pos = elem->gradientIndex % delay;
            colIndex = elem->gradientIndex / delay;

            if (elem->grad == PULSE)
            {
                colVal = (int) (255.0 * sin(PI * colIndex / numOfColors));

                const gcn::Color &col = elem->testColor;

                elem->color.r = ((colVal * col.r) / 255) % (col.r + 1);
                elem->color.g = ((colVal * col.g) / 255) % (col.g + 1);
                elem->color.b = ((colVal * col.b) / 255) % (col.b + 1);
            }
            if (elem->grad == SPECTRUM)
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

                startColVal = (cos(PI * pos / delay) + 1) / 2;
                destColVal = 1 - startColVal;

                elem->color.r =(int)(startColVal * startCol.r +
                                               destColVal * destCol.r);

                elem->color.g =(int)(startColVal * startCol.g +
                                               destColVal * destCol.g);

                elem->color.b =(int)(startColVal * startCol.b +
                                               destColVal * destCol.b);
            }
        }

        if (advance)
            mRainbowTime = tick_time;
    }
}
