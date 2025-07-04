/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
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

#include "resources/dye.h"

#include "log.h"

#include <cmath>
#include <sstream>

DyePalette::DyePalette(const std::string &description)
{
    int size = description.length();
    if (size == 0)
        return;
    if (description[0] != '#')
    {
        // TODO: load palette from file.
        return;
    }

    int pos = 1;
    for (;;)
    {
        if (pos + 6 > size)
            break;

        int v = 0;
        for (int i = 0; i < 6; ++i)
        {
            char c = description[pos + i];
            int n;

            if ('0' <= c && c <= '9')
            {
                n = c - '0';
            }
            else if ('A' <= c && c <= 'F')
            {
                n = c - 'A' + 10;
            }
            else if ('a' <= c && c <= 'f')
            {
                n = c - 'a' + 10;
            }
            else
            {
                Log::info("Error, invalid embedded palette: %s",
                          description.c_str());
                return;
            }

            v = (v << 4) | n;
        }
        Color c = { (unsigned char) (v >> 16), (unsigned char) (v >> 8), (unsigned char) v };
        mColors.push_back(c);
        pos += 6;

        if (pos == size)
            return;
        if (description[pos] != ',')
            break;

        ++pos;
    }

    Log::info("Error, invalid embedded palette: %s", description.c_str());
}

void DyePalette::getColor(int intensity, int color[3]) const
{
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
    int r2 = mColors[j].r,
        g2 = mColors[j].g,
        b2 = mColors[j].b;

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
        r1 = mColors[i - 1].r;
        g1 = mColors[i - 1].g;
        b1 = mColors[i - 1].b;
    }

    // Perform a linear interpolation.
    color[0] = ((255 - t) * r1 + t * r2) / 255;
    color[1] = ((255 - t) * g1 + t * g2) / 255;
    color[2] = ((255 - t) * b1 + t * b2) / 255;
}

void DyePalette::getColor(double intensity, int color[3]) const
{
    // Nothing to do here
    if (mColors.empty())
        return;

    // Force range
    if (intensity > 1.0)
        intensity = 1.0;
    else if (intensity < 0.0)
        intensity = 0.0;

    // Scale up
    intensity = intensity * (mColors.size() - 1);

    // Color indices
    int i = (int) floor(intensity);
    int j = (int) ceil(intensity);

    if (i == j)
    {
        // Exact color.
        color[0] = mColors[i].r;
        color[1] = mColors[i].g;
        color[2] = mColors[i].b;
        return;
    }

    intensity -= i;
    double rest = 1 - intensity;

    // Get the colors
    int r1 = mColors[i].r,
        g1 = mColors[i].g,
        b1 = mColors[i].b,
        r2 = mColors[j].r,
        g2 = mColors[j].g,
        b2 = mColors[j].b;

    // Perform the interpolation.
    color[0] = (rest * r1 + intensity * r2);
    color[1] = (rest * g1 + intensity * g2);
    color[2] = (rest * b1 + intensity * b2);
}

Dye::Dye(const std::string &description)
{
    for (auto &dyePalette : mDyePalettes)
        dyePalette = nullptr;

    if (description.empty())
        return;

    std::string::size_type next_pos = 0, length = description.length();
    do
    {
        std::string::size_type pos = next_pos;
        next_pos = description.find(';', pos);

        if (next_pos == std::string::npos)
            next_pos = length;

        if (next_pos <= pos + 3 || description[pos + 1] != ':')
        {
            Log::info("Error, invalid dye: %s", description.c_str());
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
                Log::info("Error, invalid dye: %s", description.c_str());
                return;
        }
        mDyePalettes[i] = new DyePalette(description.substr(pos + 2,
                                                            next_pos - pos - 2));
        ++next_pos;
    }
    while (next_pos < length);
}

Dye::~Dye()
{
    for (auto &dyePalette : mDyePalettes)
        delete dyePalette;
}

void Dye::update(int color[3]) const
{
    int cmax = std::max(color[0], std::max(color[1], color[2]));
    if (cmax == 0)
        return;

    int cmin = std::min(color[0], std::min(color[1], color[2]));
    int intensity = color[0] + color[1] + color[2];

    if (cmin != cmax &&
        (cmin != 0 || (intensity != cmax && intensity != 2 * cmax)))
    {
        // not pure
        return;
    }

    int i = (color[0] != 0) | ((color[1] != 0) << 1) | ((color[2] != 0) << 2);

    if (mDyePalettes[i - 1])
        mDyePalettes[i - 1]->getColor(cmax, color);
}

void Dye::instantiate(std::string &target, const std::string &palettes)
{
    std::string::size_type next_pos = target.find('|');

    if (next_pos == std::string::npos || palettes.empty())
        return;

    ++next_pos;

    std::ostringstream s;
    s << target.substr(0, next_pos);
    std::string::size_type last_pos = target.length(), pal_pos = 0;
    do
    {
        std::string::size_type pos = next_pos;
        next_pos = target.find(';', pos);

        if (next_pos == std::string::npos)
            next_pos = last_pos;

        if (next_pos == pos + 1 && pal_pos != std::string::npos)
        {
            std::string::size_type pal_next_pos = palettes.find(';', pal_pos);
            s << target[pos] << ':';
            if (pal_next_pos == std::string::npos)
            {
                s << palettes.substr(pal_pos);
                s << target.substr(next_pos);
                pal_pos = std::string::npos;
                break;
            }
            s << palettes.substr(pal_pos, pal_next_pos - pal_pos);
            pal_pos = pal_next_pos + 1;
        }
        else if (next_pos > pos + 2)
        {
            s << target.substr(pos, next_pos - pos);
        }
        else
        {
            Log::info("Error, invalid dye placeholder: %s", target.c_str());
            return;
        }
        s << target[next_pos];
        ++next_pos;
    }
    while (next_pos < last_pos);

    target = s.str();
}
