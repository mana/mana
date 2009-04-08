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

#include <algorithm>
#include <cstring>
#include <physfs.h>

#include "resources/wallpaper.h"

#include "log.h"

#include "utils/strprintf.h"

#define WALLPAPER_FOLDER "graphics/images/"

#define WALLPAPER_BASE "login_wallpaper"

struct wallpaper {
    Uint16 width;
    Uint16 height;
};

std::vector<struct wallpaper> wallpapers;
bool haveBackup; // Is the backup (no size given) version availabnle?

bool wallpaperCompare(struct wallpaper x, struct wallpaper y)
{
    int aX = x.width * x.height;
    int aY = y.width * y.height;

    if (aX > aY || (aX == aY && x.width > y.width)) return true;
    return false;
}

void Wallpaper::loadWallpapers()
{
    char **imgs = PHYSFS_enumerateFiles(WALLPAPER_FOLDER);
    char **i;
    size_t baseLen = strlen(WALLPAPER_BASE);
    int width;
    int height;

    wallpapers.clear();

    haveBackup = false;

    for (i = imgs; *i != NULL; i++)
    {
        if (strncmp(*i, WALLPAPER_BASE, baseLen) == 0)
        {
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
                struct wallpaper wp;
                wp.width = width;
                wp.height = height;
                wallpapers.push_back(wp);
            }
        }
    }

    PHYSFS_freeList(imgs);

    std::sort(wallpapers.begin(), wallpapers.end(), wallpaperCompare);
}

std::string Wallpaper::getWallpaper(int width, int height)
{
    std::vector<wallpaper>::iterator iter;
    wallpaper wp;

    for(iter = wallpapers.begin(); iter != wallpapers.end(); iter++)
    {
        wp = *iter;
        if (wp.width <= width && wp.height <= height)
            return std::string(strprintf(WALLPAPER_FOLDER WALLPAPER_BASE
                                "_%dx%d.png", wp.width, wp.height));
    }

    if (haveBackup) return std::string(WALLPAPER_FOLDER WALLPAPER_BASE ".png");

    return std::string("");
}
