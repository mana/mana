/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
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

#include <algorithm>
#include <sstream>
#include <iostream>

#include "dye.h"

Palette::Palette(const std::string &description)
{
    mLoaded = false;
    int size = description.length();
    if (size == 0) return;
    if (description[0] != '#')
    {
        std::cout << "Missing # in the palette description "
        << "in the third parameter." << std::endl;
        return;
    }

    int pos = 1;
    for (;;)
    {
        if (pos + 6 > size) break;
        int v = 0;
        for (int i = 0; i < 6; ++i)
        {
            char c = description[pos + i];
            int n;
            if ('0' <= c && c <= '9')
                n = c - '0';
            else if ('A' <= c && c <= 'F')
                n = c - 'A' + 10;
            else if ('a' <= c && c <= 'f')
                n = c - 'a' + 10;
            else
            {
                std::cout << "invalid Hexadecimal description: "
                << description << std::endl;
                return;
            }

            v = (v << 4) | n;
        }
        Color c = { { v >> 16, v >> 8, v } };
        mColors.push_back(c);
        pos += 6;
        if (pos == size)
        {
            mLoaded = true;
            return;
        }
        if (description[pos] != ',')
            break;

        ++pos;
    }

    mLoaded = true;
}

void Palette::getColor(int intensity, int color[3]) const
{
    // Return implicit black
    if (intensity == 0)
    {
        color[0] = 0;
        color[1] = 0;
        color[2] = 0;
        return;
    }

    int last = mColors.size();
    if (last == 0) return;

    int i = intensity * last / 255;
    int t = intensity * last % 255;

    int j = t != 0 ? i : i - 1;
    // Get the exact color if any, the next color otherwise.
    int r2 = mColors[j].value[0],
        g2 = mColors[j].value[1],
        b2 = mColors[j].value[2];

    if (t == 0)
    {
        // Exact color.
        color[0] = r2;
        color[1] = g2;
        color[2] = b2;
        return;
    }

    // Get the previous color. First color is implicitly black.
    int r1 = 0, g1 = 0, b1 = 0;
    if (i > 0)
    {
        r1 = mColors[i - 1].value[0];
        g1 = mColors[i - 1].value[1];
        b1 = mColors[i - 1].value[2];
    }

    // Perform a linear interpolation.
    color[0] = ((255 - t) * r1 + t * r2) / 255;
    color[1] = ((255 - t) * g1 + t * g2) / 255;
    color[2] = ((255 - t) * b1 + t * b2) / 255;
}

Dye::Dye(const std::string &description)
{
    mLoaded = false;
    for (int i = 0; i < 7; ++i)
        mPalettes[i] = 0;

    if (description.empty()) return;

    std::string::size_type next_pos = 0, length = description.length();
    do
    {
        std::string::size_type pos = next_pos;
        next_pos = description.find(';', pos);
        if (next_pos == std::string::npos)
            next_pos = length;
        if (next_pos <= pos + 3 || description[pos + 1] != ':')
        {
            std::cout << "Dyeing: Missing ':' in channel description."
            << std::endl;
            return;
        }
        int i = 0;
        switch (description[pos])
        {
            case 'R': i = 0; break;
            case 'G': i = 1; break;
            case 'Y': i = 2; break;
            case 'B': i = 3; break;
            case 'M': i = 4; break;
            case 'C': i = 5; break;
            case 'W': i = 6; break;
            default:
                std::cout << "Dyeing: Invalid channel. Not in [R,G,Y,B,M,C,W]"
                << std::endl;
                return;
        }
        mPalettes[i] = new Palette(
                           description.substr(pos + 2, next_pos - pos - 2));

        if (!mPalettes[i]->loaded())
            return;

        ++next_pos;
    }
    while (next_pos < length);

    mLoaded = true;
}

Dye::~Dye()
{
    for (int i = 0; i < 7; ++i)
        delete mPalettes[i];
}

void Dye::update(int color[3]) const
{
    int cmax = std::max(color[0], std::max(color[1], color[2]));
    if (cmax == 0) return;

    int cmin = std::min(color[0], std::min(color[1], color[2]));
    int intensity = color[0] + color[1] + color[2];

    if (cmin != cmax &&
        (cmin != 0 || (intensity != cmax && intensity != 2 * cmax)))
    {
        // not pure
        return;
    }

    int i = (color[0] != 0) | ((color[1] != 0) << 1) | ((color[2] != 0) << 2);

    if (mPalettes[i - 1])
        mPalettes[i - 1]->getColor(cmax, color);
}
