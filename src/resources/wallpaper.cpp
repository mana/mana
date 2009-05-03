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
#define WALLPAPER_BASE   "login_wallpaper"

struct WallpaperSize
{
    int width;
    int height;
};

static std::vector<WallpaperSize> wallpaperSizes;
static bool haveBackup; // Is the backup (no size given) version available?

bool wallpaperCompare(WallpaperSize a, WallpaperSize b)
{
    int aa = a.width * a.height;
    int ab = b.width * b.height;

    return (aa > ab || (aa == ab && a.width > b.width));
}

void Wallpaper::loadWallpapers()
{
    wallpaperSizes.clear();

    size_t baseLen = strlen(WALLPAPER_BASE);
    haveBackup = false;

    char **imgs = PHYSFS_enumerateFiles(WALLPAPER_FOLDER);

    for (char **i = imgs; *i != NULL; i++)
    {
        if (strncmp(*i, WALLPAPER_BASE, baseLen) == 0)
        {
            int width;
            int height;

            if (strlen(*i) == baseLen + 4)
            {
                if (haveBackup)
                    logger->log("Duplicate default wallpaper!");
                else
                    haveBackup = true;
            }
            else if (sscanf(*i, WALLPAPER_BASE "_%dx%d.png",
                            &width, &height) == 2)
            {
                WallpaperSize wp;
                wp.width = width;
                wp.height = height;
                wallpaperSizes.push_back(wp);
            }
        }
    }

    PHYSFS_freeList(imgs);

    std::sort(wallpaperSizes.begin(), wallpaperSizes.end(), wallpaperCompare);
}

std::string Wallpaper::getWallpaper(int width, int height)
{
    std::vector<WallpaperSize>::iterator iter;
    WallpaperSize wp;

    for (iter = wallpaperSizes.begin(); iter != wallpaperSizes.end(); iter++)
    {
        wp = *iter;
        if (wp.width <= width && wp.height <= height)
        {
            return std::string(strprintf(WALLPAPER_FOLDER WALLPAPER_BASE
                                         "_%dx%d.png", wp.width, wp.height));
        }
    }

    if (haveBackup)
        return std::string(WALLPAPER_FOLDER WALLPAPER_BASE ".png");

    return std::string();
}
