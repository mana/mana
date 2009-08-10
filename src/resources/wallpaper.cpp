/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "resources/wallpaper.h"

#include "log.h"

#include "utils/stringutils.h"

#include <algorithm>
#include <cstring>
#include <vector>

#include <physfs.h>

#define WALLPAPER_FOLDER "graphics/images/"
#define WALLPAPER_BASE   "login_wallpaper.png"

struct WallpaperData
{
    std::string filename;
    int width;
    int height;
};

static std::vector<WallpaperData> wallpaperData;
static bool haveBackup; // Is the backup (no size given) version available?

bool wallpaperCompare(WallpaperData a, WallpaperData b)
{
    int aa = a.width * a.height;
    int ab = b.width * b.height;

    return (aa > ab || (aa == ab && a.width > b.width));
}

void Wallpaper::loadWallpapers()
{
    wallpaperData.clear();

    char **imgs = PHYSFS_enumerateFiles(WALLPAPER_FOLDER);

    for (char **i = imgs; *i != NULL; i++)
    {
        int width;
        int height;

        // If the backup file is found, we tell it.
        if (strncmp (*i, WALLPAPER_BASE, strlen(*i)) == 0)
            haveBackup = true;

        // If the image format is terminated by: "_<width>x<height>.png"
        // It is taken as a potential wallpaper.

        // First, get the base filename of the image:
        std::string filename = *i;
        int separator = filename.rfind("_");
        filename = filename.substr(0, separator);

        // Then, append the width and height search mask.
        filename.append("_%dx%d.png");

        if (sscanf(*i, filename.c_str(), &width, &height) == 2)
        {
            WallpaperData wp;
            wp.filename = WALLPAPER_FOLDER;
            wp.filename.append(*i);
            wp.width = width;
            wp.height = height;
            wallpaperData.push_back(wp);
        }
    }

    PHYSFS_freeList(imgs);

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
          srand((unsigned)time(0)); 
          return wallPaperVector
          [int(wallPaperVector.size() * rand() / (RAND_MAX + 1.0))];
        }
        else // If there at least one, we return it
          return wallPaperVector[0];
    }

    // Return the backup file if everything else failed...
    if (haveBackup)
        return std::string(WALLPAPER_FOLDER WALLPAPER_BASE);

    // Return an empty string if everything else failed
    return std::string();

}
