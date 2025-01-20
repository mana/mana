/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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

#include "defaults.h"

#include "client.h"

VariableData *createData(int defData)
{
    return new IntData(defData);
}

VariableData *createData(double defData)
{
    return new FloatData(defData);
}

VariableData *createData(float defData)
{
    return new FloatData(defData);
}

VariableData *createData(const std::string &defData)
{
    return new StringData(defData);
}

VariableData *createData(const char* defData)
{
    return new StringData(defData);
}

VariableData *createData(bool defData)
{
    return new BoolData(defData);
}

template<typename T>
void AddDEF(DefaultsData *defaultsData, const char *key, T value)
{
    defaultsData->insert(std::make_pair(key, createData(value)));
}

DefaultsData* getBrandingDefaults()
{
    auto *brandingData = new DefaultsData;
    // Init config defaults
    AddDEF(brandingData, "wallpapersPath", "");
    AddDEF(brandingData, "wallpaperFile", "");
    AddDEF(brandingData, "appName", "Mana");
    AddDEF(brandingData, "appIcon", "icons/mana");
    AddDEF(brandingData, "loginMusic", "system/Magick - Real.ogg");
    AddDEF(brandingData, "defaultServer", "");
    AddDEF(brandingData, "defaultPort", DEFAULT_PORT);
    AddDEF(brandingData, "defaultServerType", "tmwathena");
    AddDEF(brandingData, "appShort", "mana");
    AddDEF(brandingData, "defaultUpdateHost", "");
    AddDEF(brandingData, "helpPath", "");
    AddDEF(brandingData, "onlineServerList", "");
    AddDEF(brandingData, "guiThemePath", "");
    AddDEF(brandingData, "theme", "");
    AddDEF(brandingData, "font", "fonts/dejavusans.ttf");
    AddDEF(brandingData, "boldFont", "fonts/dejavusans-bold.ttf");
    AddDEF(brandingData, "monoFont", "fonts/dejavusans-mono.ttf");

    return brandingData;
}

DefaultsData* getPathsDefaults()
{
    auto *pathsData = new DefaultsData;
    // Init paths.xml defaults
    AddDEF(pathsData, "itemIcons", "graphics/items/");
    AddDEF(pathsData, "unknownItemFile", "unknown-item.png");
    AddDEF(pathsData, "sprites", "graphics/sprites/");
    AddDEF(pathsData, "spriteErrorFile", "error.xml");

    AddDEF(pathsData, "particles", "graphics/particles/");
    AddDEF(pathsData, "levelUpEffectFile", "levelup.particle.xml");
    AddDEF(pathsData, "portalEffectFile", "warparea.particle.xml");
    AddDEF(pathsData, "hitEffectId", 26);
    AddDEF(pathsData, "criticalHitEffectId", 28);

    AddDEF(pathsData, "minimaps", "graphics/minimaps/");
    AddDEF(pathsData, "maps", "maps/");

    AddDEF(pathsData, "sfx", "sfx/");
    AddDEF(pathsData, "attackSfxFile", "fist-swish.ogg");
    AddDEF(pathsData, "music", "music/");

    AddDEF(pathsData, "wallpapers", "graphics/images/");
    AddDEF(pathsData, "wallpaperFile", "login_wallpaper.png");

    AddDEF(pathsData, "help", "help/");

    return pathsData;
}

#undef AddDEF
