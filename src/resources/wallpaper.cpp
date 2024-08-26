/*
 *  The Mana Client
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

#include "resources/wallpaper.h"

#include "configuration.h"

#include "utils/filesystem.h"

#include <algorithm>
#include <cstring>
#include <vector>

struct WallpaperData
{
    std::string filename;
    int width;
    int height;
};

inline std::ostream& operator <<(std::ostream &os, const WallpaperData &d)
{
    os << d.filename << "[" << d.width << "x" << d.height << "]";
    return os;
}

static std::vector<WallpaperData> wallpaperData;
static bool haveBackup; // Is the backup (no size given) version available?

static std::string wallpaperPath;
static std::string wallpaperFile;

static void initWallpaperPaths()
{
    // Init the path
    wallpaperPath = paths.getStringValue("wallpapers");

    if (wallpaperPath.empty())
    {
        wallpaperPath = branding.getStringValue("wallpapersPath");

        if (wallpaperPath.empty())
            wallpaperPath = "graphics/images/";
    }

    // Init the default file
    wallpaperFile = paths.getStringValue("wallpaperFile");

    if (wallpaperFile.empty())
    {
        wallpaperFile = branding.getStringValue("wallpaperFile");

        if (wallpaperFile.empty())
            wallpaperFile = "login_wallpaper.png";
    }
}

/**
 * Comparison function that puts the largest wallpaper first.
 */
bool wallpaperCompare(const WallpaperData &a, const WallpaperData &b)
{
    int aa = a.width * a.height;
    int ab = b.width * b.height;

    return (aa > ab || (aa == ab && a.width > b.width));
}

void Wallpaper::loadWallpapers()
{
    wallpaperData.clear();

    initWallpaperPaths();

    for (auto fileName : FS::enumerateFiles(wallpaperPath))
    {
        // If the backup file is found, we tell it.
        if (wallpaperFile == fileName)
            haveBackup = true;

        // If the image format is terminated by: "_<width>x<height>.png"
        // It is taken as a potential wallpaper.
        if (auto sizeSuffix = strrchr(fileName, '_'))
        {
            int width;
            int height;

            if (sscanf(sizeSuffix, "_%dx%d.png", &width, &height) == 2)
            {
                WallpaperData wp;
                wp.filename = wallpaperPath;
                wp.filename.append(fileName);
                wp.width = width;
                wp.height = height;
                wallpaperData.push_back(wp);
            }
        }
    }

    std::sort(wallpaperData.begin(), wallpaperData.end(), wallpaperCompare);
}

std::string Wallpaper::getWallpaper(int width, int height)
{
    if (wallpaperData.empty())
        return haveBackup ? (wallpaperPath + wallpaperFile) : std::string();

    WallpaperData wallpaper;

    // Search for the smallest wallpaper at least as large as the screen
    for (auto &wp : wallpaperData)
    {
        if (wp.width >= width && wp.height >= height)
        {
            if (wallpaper.filename.empty() || (wallpaper.width < wp.width &&
                                               wallpaper.height < wp.height))
            {
                wallpaper = wp;
            }
        }
    }

    // When no fitting wallpaper was found yet, pick the biggest one
    if (wallpaper.filename.empty())
    {
        wallpaper = wallpaperData.front();
    }

    return wallpaper.filename;
}
