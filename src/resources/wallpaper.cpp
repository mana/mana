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
    std::vector<WallpaperData>::iterator iter;
    WallpaperData wp;

    // Wallpaper filename container
    std::vector<std::string> wallPaperVector;

    for (iter = wallpaperData.begin(); iter != wallpaperData.end(); iter++)
    {
        wp = *iter;
        if (wp.width <= width && wp.height <= height)
            wallPaperVector.push_back(wp.filename);
    }

    if (!wallPaperVector.empty())
    {
        // If we've got more than one occurence of a valid wallpaper...
        if (wallPaperVector.size() > 0)
        {
          // Return randomly a wallpaper between vector[0] and
          // vector[vector.size() - 1]
          srand((unsigned) time(0));
          return wallPaperVector
          [int(wallPaperVector.size() * rand() / (RAND_MAX + 1.0))];
        }
        else // If there at least one, we return it
          return wallPaperVector[0];
    }

    // Return the backup file if everything else failed...
    if (haveBackup)
        return std::string(wallpaperPath + wallpaperFile);

    // Return an empty string if everything else failed
    return std::string();
}
