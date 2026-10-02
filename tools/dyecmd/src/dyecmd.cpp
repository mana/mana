/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_image.h>

#include "resources/dye.h"

using namespace std;

// return values
enum ReturnValues
{
    RETURN_OK = 0,
    INVALID_PARAMETER_LIST = 100,
    INVALID_INPUT_IMAGE = 101,
    INVALID_OUTPUT_IMAGE = 102,
    INVALID_DYE_PARAMETER = 105
};

/**
 * Loads an image and converts it to the same pixel format the client uses
 * before dyeing.
 */
static SDL_Surface *loadImage(const std::string &filename)
{
    SDL_Surface *loaded = IMG_Load(filename.c_str());
    if (!loaded)
        return nullptr;

    SDL_Surface *surface = SDL_ConvertSurfaceFormat(
        loaded, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(loaded);
    return surface;
}

/**
 * Applies a dye to a surface exactly like Image::load does. Returns the
 * number of changed pixels.
 */
static int recolor(SDL_Surface *surface, const Dye &dye)
{
    if (SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);

    int changed = 0;
    SDL_Color *pixels = static_cast<SDL_Color *>(surface->pixels);
    SDL_Color *const end = pixels + surface->w * surface->h;
    for (SDL_Color *p = pixels; p != end; ++p)
    {
        if (p->a == 0)
            continue;
        int color[4] = {p->r, p->g, p->b, p->a};
        dye.update(color);
        if (color[0] != p->r || color[1] != p->g || color[2] != p->b ||
            color[3] != p->a)
        {
            ++changed;
        }
        p->r = color[0];
        p->g = color[1];
        p->b = color[2];
        p->a = color[3];
    }

    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);
    return changed;
}

/**
 * Counts, per intensity channel, how many pixels of the surface are
 * eligible for recoloring. Applies the same purity rules as Dye::update.
 */
static void countChannelPixels(SDL_Surface *surface, int count[7])
{
    SDL_Color *pixels = static_cast<SDL_Color *>(surface->pixels);
    SDL_Color *const end = pixels + surface->w * surface->h;
    for (SDL_Color *p = pixels; p != end; ++p)
    {
        if (p->a == 0)
            continue;

        const int cmax = std::max(p->r, std::max(p->g, p->b));
        if (cmax == 0)
            continue;

        const int cmin = std::min(p->r, std::min(p->g, p->b));
        const int intensity = p->r + p->g + p->b;
        if (cmin != cmax &&
            (cmin != 0 || (intensity != cmax && intensity != 2 * cmax)))
        {
            continue;
        }

        const int i =
            (p->r != 0) | ((p->g != 0) << 1) | ((p->b != 0) << 2);
        ++count[i - 1];
    }
}

/**
 * Counts how many pixels match each "from" color of an S or A palette.
 * The palette is a comma separated list of hex colors (6 digits for S,
 * 8 for A); even and odd entries form (from, to) pairs.
 */
static void countPairMatches(SDL_Surface *surface,
                             const std::string &palette,
                             bool withAlpha,
                             std::vector<std::pair<std::string, int>> &matches)
{
    std::map<uint32_t, int> pixelCount;
    SDL_Color *pixels = static_cast<SDL_Color *>(surface->pixels);
    SDL_Color *const end = pixels + surface->w * surface->h;
    for (SDL_Color *p = pixels; p != end; ++p)
    {
        if (p->a == 0)
            continue;
        const uint32_t key = withAlpha
            ? (p->r << 24) | (p->g << 16) | (p->b << 8) | p->a
            : (p->r << 16) | (p->g << 8) | p->b;
        ++pixelCount[key];
    }

    const int digits = withAlpha ? 8 : 6;
    std::vector<std::string> colors;
    for (std::string::size_type pos = 1, size = palette.size();
         pos + digits <= size; pos += digits + 1)
    {
        if (pos > 1 && palette[pos - 1] != ',')
            break;
        colors.push_back(palette.substr(pos, digits));
    }

    matches.clear();
    for (std::size_t i = 0; i + 1 < colors.size(); i += 2)
    {
        const uint32_t from = strtoul(colors[i].c_str(), nullptr, 16);
        matches.emplace_back(colors[i], pixelCount[from]);
    }
}

static int doAnalyze(const std::string &imagePath, const std::string &spec)
{
    SDL_Surface *surface = loadImage(imagePath);
    if (!surface)
    {
        cout << INVALID_INPUT_IMAGE << " - INVALID_INPUT_IMAGE: "
             << imagePath << endl;
        return INVALID_INPUT_IMAGE;
    }

    int channelCount[7] = {};
    countChannelPixels(surface, channelCount);
    static const char names[] = "RGYBMCW";

    cout << imagePath << ": " << surface->w << "x" << surface->h << endl;
    if (spec.empty())
    {
        for (int i = 0; i < 7; ++i)
            cout << "  " << names[i] << " channel pixels: "
                 << channelCount[i] << endl;
        SDL_FreeSurface(surface);
        return RETURN_OK;
    }

    std::string::size_type pos = 0, end = spec.size();
    while (pos < end)
    {
        const std::string::size_type next = spec.find(';', pos);
        const std::string segment = spec.substr(
            pos, next == std::string::npos ? end - pos : next - pos);
        pos = next == std::string::npos ? end : next + 1;

        if (segment.empty())
            continue;
        const char channel = segment[0];
        const char *idx = strchr(names, channel);
        const bool hasPalette =
            segment.size() > 2 && segment[1] == ':' && segment[2] == '#';

        if (channel == 'S' || channel == 'A')
        {
            if (!hasPalette)
            {
                cout << "  " << channel << "  placeholder" << endl;
                continue;
            }
            std::vector<std::pair<std::string, int>> matches;
            countPairMatches(surface, segment.substr(2), channel == 'A',
                             matches);
            int total = 0;
            for (auto &m : matches)
            {
                total += m.second;
                cout << "  " << channel << "  #" << m.first << " -> "
                     << m.second << " pixels" << endl;
            }
            cout << "  " << channel << "  total: " << total
                 << (total ? "" : "  (no matching pixels)") << endl;
        }
        else if (idx)
        {
            const int n = channelCount[idx - names];
            cout << "  " << channel << (hasPalette ? "" : "  placeholder")
                 << "  " << n << " pixels"
                 << (n ? "" : "  (no matching pixels)") << endl;
        }
        else
        {
            cout << "  " << channel << "  unknown channel" << endl;
        }
    }

    SDL_FreeSurface(surface);
    return RETURN_OK;
}

static void printHelp()
{
    cout << endl <<
"This tool is used to dye item graphics used by the Mana client.\n"
"See docs/dye.md for the dye format.\n\n"
"Usage:\n"
"  dyecmd <source> <target> <dye>           dye source image to target\n"
"  dyecmd <source|dye> <target>             same, dye embedded after '|'\n"
"  dyecmd analyze <image> [dye]             count pixels affected\n"
"  dyecmd instantiate <target> <palettes>   print target after filling "
"placeholders\n\n"
"e.g.:\n"
"  dyecmd armor-legs-shorts.png armor-legs-shorts2.png \"W:#222255,6666ff\"\n"
"  dyecmd \"armor-legs-shorts.png|W:#222255,6666ff\" armor-legs-shorts2.png\n"
"  dyecmd analyze santahat.png \"S:#780000,3a3a3a\"\n"
         << endl;
}

int main(int argc, char *argv[])
{
    ReturnValues returnValue = RETURN_OK;

    if (argc > 1 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")))
    {
        printHelp();
        return RETURN_OK;
    }

    SDL_Init(0);

    if (argc == 4 && !strcmp(argv[1], "analyze"))
    {
        returnValue = static_cast<ReturnValues>(doAnalyze(argv[2], argv[3]));
    }
    else if (argc == 3 && !strcmp(argv[1], "analyze"))
    {
        returnValue = static_cast<ReturnValues>(doAnalyze(argv[2], ""));
    }
    else if (argc == 4 && !strcmp(argv[1], "instantiate"))
    {
        std::string target = argv[2];
        Dye::instantiate(target, argv[3]);
        cout << target << endl;
    }
    else
    {
        // Split off an embedded "|dye" suffix or take the dye argument.
        std::string inputFile, outputFile, dyeDescription;
        if (argc == 4)
        {
            inputFile = argv[1];
            outputFile = argv[2];
            dyeDescription = argv[3];
        }
        else if (argc == 3)
        {
            inputFile = argv[1];
            outputFile = argv[2];
            const std::string::size_type pipe = inputFile.find('|');
            if (pipe == std::string::npos)
            {
                cout << INVALID_PARAMETER_LIST << " - INVALID_PARAMETER_LIST";
                printHelp();
                return INVALID_PARAMETER_LIST;
            }
            dyeDescription = inputFile.substr(pipe + 1);
            inputFile.erase(pipe);
        }
        else
        {
            cout << INVALID_PARAMETER_LIST << " - INVALID_PARAMETER_LIST";
            printHelp();
            return INVALID_PARAMETER_LIST;
        }

        Dye dye(dyeDescription);
        SDL_Surface *source = loadImage(inputFile);
        if (!source)
        {
            cout << INVALID_INPUT_IMAGE << " - INVALID_INPUT_IMAGE: "
                 << inputFile << endl;
            returnValue = INVALID_INPUT_IMAGE;
        }
        else
        {
            const int changed = recolor(source, dye);
            if (IMG_SavePNG(source, outputFile.c_str()) != 0)
            {
                cout << INVALID_OUTPUT_IMAGE << " - INVALID_OUTPUT_IMAGE: "
                     << outputFile << " (" << IMG_GetError() << ")" << endl;
                returnValue = INVALID_OUTPUT_IMAGE;
            }
            else
            {
                cout << changed << " pixels changed" << endl;
            }
            SDL_FreeSurface(source);
        }
    }

    SDL_Quit();
    return returnValue;
}
