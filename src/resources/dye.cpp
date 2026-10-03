/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include <algorithm>
#include <cmath>
#include <sstream>

DyePalette::DyePalette(const std::string &description, bool withAlpha) :
    mWithAlpha(withAlpha)
{
    int size = description.length();
    if (size == 0)
        return;
    if (description[0] != '#')
    {
        // TODO: load palette from file.
        return;
    }

    const int digits = withAlpha ? 8 : 6;
    int pos = 1;
    for (;;)
    {
        if (pos + digits > size)
            break;

        int v = 0;
        for (int i = 0; i < digits; ++i)
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
        SDL_Color c;
        if (withAlpha)
        {
            c.r = (unsigned char) (v >> 24);
            c.g = (unsigned char) (v >> 16);
            c.b = (unsigned char) (v >> 8);
            c.a = (unsigned char) v;
        }
        else
        {
            c.r = (unsigned char) (v >> 16);
            c.g = (unsigned char) (v >> 8);
            c.b = (unsigned char) v;
            c.a = 255;
        }
        mColors.push_back(c);
        pos += digits;

        if (pos == size)
        {
            for (int i = 0; i < 256; ++i)
                computeColor(i, mIntensityLut[i]);
            return;
        }
        if (description[pos] != ',')
            break;

        ++pos;
    }

    Log::info("Error, invalid embedded palette: %s", description.c_str());
}

void DyePalette::computeColor(int intensity, SDL_Color &color) const
{
    if (intensity == 0)
    {
        color = {0, 0, 0, 255};
        return;
    }

    const int last = mColors.size();
    int i = intensity * last / 255;
    int t = intensity * last % 255;

    int j = t != 0 ? i : i - 1;
    // Get the exact color if any, the next color otherwise.
    const SDL_Color &c2 = mColors[j];

    if (t == 0)
    {
        // Exact color.
        color = c2;
        return;
    }

    // Get the previous color. First color is implicitly black.
    SDL_Color c1 = {0, 0, 0, 255};
    if (i > 0)
        c1 = mColors[i - 1];

    // Perform a linear interpolation.
    color.r = ((255 - t) * c1.r + t * c2.r) / 255;
    color.g = ((255 - t) * c1.g + t * c2.g) / 255;
    color.b = ((255 - t) * c1.b + t * c2.b) / 255;
}

void DyePalette::getColor(int intensity, SDL_Color &color) const
{
    if (mColors.empty())
        return;

    const SDL_Color &c = mIntensityLut[std::clamp(intensity, 0, 255)];
    color.r = c.r;
    color.g = c.g;
    color.b = c.b;
}

void DyePalette::getColor(double intensity, SDL_Color &color) const
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
        color.r = mColors[i].r;
        color.g = mColors[i].g;
        color.b = mColors[i].b;
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
    color.r = (rest * r1 + intensity * r2);
    color.g = (rest * g1 + intensity * g2);
    color.b = (rest * b1 + intensity * b2);
}

bool DyePalette::replaceColor(SDL_Color &color) const
{
    for (std::size_t i = 0; i + 1 < mColors.size(); i += 2)
    {
        const SDL_Color &from = mColors[i];
        if (from.r == color.r && from.g == color.g && from.b == color.b &&
            (!mWithAlpha || from.a == color.a))
        {
            const SDL_Color &to = mColors[i + 1];
            color.r = to.r;
            color.g = to.g;
            color.b = to.b;
            if (mWithAlpha)
                color.a = to.a;
            return true;
        }
    }
    return false;
}

Dye::Dye(const std::string &description)
{
    if (description.empty())
        return;

    std::string::size_type next_pos = 0, length = description.length();
    do
    {
        std::string::size_type pos = next_pos;
        next_pos = description.find(';', pos);

        if (next_pos == std::string::npos)
            next_pos = length;

        int i;
        switch (description[pos])
        {
            case 'R': i = 0; break;
            case 'G': i = 1; break;
            case 'Y': i = 2; break;
            case 'B': i = 3; break;
            case 'M': i = 4; break;
            case 'C': i = 5; break;
            case 'W': i = 6; break;
            case 'S': i = 7; break;
            case 'A': i = 8; break;
            default:  i = -1; break;
        }

        if (next_pos <= pos + 3 || description[pos + 1] != ':')
        {
            // A bare channel letter is a placeholder that no palette was
            // provided for. The channel is left undyed.
            if (i >= 0 && next_pos == pos + 1)
            {
                ++next_pos;
                continue;
            }
            Log::info("Error, invalid dye: %s", description.c_str());
            return;
        }

        if (i < 0)
        {
            Log::info("Error, invalid dye: %s", description.c_str());
            return;
        }
        mDyePalettes[i] = std::make_unique<DyePalette>(
            description.substr(pos + 2, next_pos - pos - 2), i == 8);
        ++next_pos;
    }
    while (next_pos < length);
}

Dye::~Dye() = default;

void Dye::update(SDL_Color &color) const
{
    // The S and A channels replace exact colors and take precedence over
    // the intensity-based channels.
    if (mDyePalettes[7])
    {
        mDyePalettes[7]->replaceColor(color);
        return;
    }
    if (mDyePalettes[8])
    {
        mDyePalettes[8]->replaceColor(color);
        return;
    }

    int cmax = std::max(color.r, std::max(color.g, color.b));
    if (cmax == 0)
        return;

    int cmin = std::min(color.r, std::min(color.g, color.b));
    int intensity = color.r + color.g + color.b;

    if (cmin != cmax &&
        (cmin != 0 || (intensity != cmax && intensity != 2 * cmax)))
    {
        // not pure
        return;
    }

    int i = (color.r != 0) | ((color.g != 0) << 1) | ((color.b != 0) << 2);

    if (mDyePalettes[i - 1])
        mDyePalettes[i - 1]->getColor(cmax, color);
}

void Dye::instantiate(std::string &target, const std::string &palettes)
{
    std::string::size_type next_pos = target.find('|');

    if (next_pos == std::string::npos)
        return;

    ++next_pos;

    std::ostringstream s;
    s << target.substr(0, next_pos - 1);
    const std::string::size_type last_pos = target.length();
    std::string::size_type pal_pos =
        palettes.empty() ? std::string::npos : 0;
    bool wroteSeparator = false;
    do
    {
        std::string::size_type pos = next_pos;
        next_pos = target.find(';', pos);

        if (next_pos == std::string::npos)
            next_pos = last_pos;

        std::string segment;
        if (next_pos == pos + 1)
        {
            // A single letter is a placeholder filled positionally from
            // palettes. When no palette is left it is dropped.
            if (pal_pos != std::string::npos)
            {
                std::string::size_type pal_next_pos =
                    palettes.find(';', pal_pos);
                segment += target[pos];
                segment += ':';
                if (pal_next_pos == std::string::npos)
                {
                    segment += palettes.substr(pal_pos);
                    pal_pos = std::string::npos;
                }
                else
                {
                    segment += palettes.substr(pal_pos,
                                               pal_next_pos - pal_pos);
                    pal_pos = pal_next_pos + 1;
                    if (pal_pos >= palettes.size())
                        pal_pos = std::string::npos;
                }
            }
        }
        else if (next_pos > pos + 2)
        {
            segment = target.substr(pos, next_pos - pos);
        }
        else
        {
            Log::info("Error, invalid dye placeholder: %s", target.c_str());
            return;
        }

        if (!segment.empty())
        {
            s << (wroteSeparator ? ';' : '|');
            wroteSeparator = true;
            s << segment;
        }
        ++next_pos;
    }
    while (next_pos < last_pos);

    target = s.str();
}
