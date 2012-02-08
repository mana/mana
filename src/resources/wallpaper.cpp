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

#include "resources/resourcemanager.h"
#include "log.h"

#include "utils/stringutils.h"
#include "configuration.h"

#include <physfs.h>

#include <algorithm>
#include <cstring>
#include <time.h>
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

    char **fileNames = PHYSFS_enumerateFiles(wallpaperPath.c_str());

    for (char **fileName = fileNames; *fileName; fileName++)
    {
        int width;
        int height;

        // If the backup file is found, we tell it.
        if (strncmp(*fileName, wallpaperFile.c_str(), strlen(*fileName)) == 0)
            haveBackup = true;

        // If the image format is terminated by: "_<width>x<height>.png"
        // It is taken as a potential wallpaper.

        // First, get the base filename of the image:
        std::string filename = *fileName;
        filename = filename.substr(0, filename.rfind("_"));

        // Check that the base filename doesn't have any '%' markers.
        if (filename.find("%") == std::string::npos)
        {
            // Then, append the width and height search mask.
            filename.append("_%dx%d.png");

            if (sscanf(*fileName, filename.c_str(), &width, &height) == 2)
            {
                WallpaperData wp;
                wp.filename = wallpaperPath;
                wp.filename.append(*fileName);
                wp.width = width;
                wp.height = height;
                wallpaperData.push_back(wp);
            }
        }
    }

    PHYSFS_freeList(fileNames);

    std::sort(wallpaperData.begin(), wallpaperData.end(), wallpaperCompare);
}

std::string Wallpaper::getWallpaper(int width, int height)
{
    if (wallpaperData.empty())
        return haveBackup ? (wallpaperPath + wallpaperFile) : std::string();

    WallpaperData wallpaper;

    // Search for the smallest wallpaper at least as large as the screen
    std::vector<WallpaperData>::iterator iter;
    for (iter = wallpaperData.begin(); iter != wallpaperData.end(); iter++)
    {
        const WallpaperData &wp = *iter;

        if (wp.width >= width && wp.height >= height)
        {
            if (wallpaper.filename.empty() ||
                    (wallpaper.width < wp.width &&
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
