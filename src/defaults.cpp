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

#include "being.h"
#include "graphics.h"
#include "client.h"

#include <stdlib.h>

VariableData* createData(int defData)
{
    return new IntData(defData);
}

VariableData* createData(double defData)
{
    return new FloatData(defData);
}

VariableData* createData(float defData)
{
    return new FloatData(defData);
}

VariableData* createData(const std::string &defData)
{
    return new StringData(defData);
}

VariableData* createData(const char* defData)
{
    return new StringData(defData);
}

VariableData* createData(bool defData)
{
    return new BoolData(defData);
}

#define AddDEF(defaultsData, key, value)  \
 defaultsData->insert(std::pair<std::string, VariableData*> \
                     (key, createData(value)));


DefaultsData* getConfigDefaults()
{
    DefaultsData* configData = new DefaultsData;
    // Init main config defaults
    AddDEF(configData, "OverlayDetail", 2);
    AddDEF(configData, "speechBubblecolor", "000000");
    AddDEF(configData, "speechBubbleAlpha", 1.0f);
    AddDEF(configData, "MostUsedServerName0", "server.themanaworld.org");
    AddDEF(configData, "visiblenames", true);
    AddDEF(configData, "speech", Being::TEXT_OVERHEAD);
    AddDEF(configData, "showgender", false);
    AddDEF(configData, "showMonstersTakedDamage", false);
    AddDEF(configData, "particleMaxCount", 3000);
    AddDEF(configData, "particleFastPhysics", 0);
    AddDEF(configData, "particleEmitterSkip", 1);
    AddDEF(configData, "particleeffects", true);
    AddDEF(configData, "logToStandardOut", false);
    AddDEF(configData, "opengl", true);
    AddDEF(configData, "screenwidth", defaultScreenWidth);
    AddDEF(configData, "screenheight", defaultScreenHeight);
    AddDEF(configData, "screen", false);
    AddDEF(configData, "hwaccel", false);
    AddDEF(configData, "sound", false);
    AddDEF(configData, "sfxVolume", 100);
    AddDEF(configData, "notificationsVolume", 100);
    AddDEF(configData, "musicVolume", 60);
    AddDEF(configData, "remember", false);
    AddDEF(configData, "username", "");
    AddDEF(configData, "lastCharacter", "");
    AddDEF(configData, "fpslimit", 60);
    AddDEF(configData, "updatehost", "");
    AddDEF(configData, "screenshotDirectory", "");
    AddDEF(configData, "useScreenshotDirectorySuffix", true);
    AddDEF(configData, "screenshotDirectorySuffix", "");
    AddDEF(configData, "EnableSync", false);
    AddDEF(configData, "joystickEnabled", false);
    AddDEF(configData, "upTolerance", 100);
    AddDEF(configData, "downTolerance", 100);
    AddDEF(configData, "leftTolerance", 100);
    AddDEF(configData, "rightTolerance", 100);
    AddDEF(configData, "download-music", false);
    AddDEF(configData, "guialpha", 0.8f);
    AddDEF(configData, "ChatLogLength", 0);
    AddDEF(configData, "enableChatLog", false);
    AddDEF(configData, "whispertab", true);
    AddDEF(configData, "customcursor", true);
    AddDEF(configData, "showownname", false);
    AddDEF(configData, "showpickupparticle", true);
    AddDEF(configData, "showpickupchat", false);
    AddDEF(configData, "fontSize", 12);
    AddDEF(configData, "ReturnToggles", false);
    AddDEF(configData, "ScrollLaziness", 16);
    AddDEF(configData, "ScrollRadius", 0);
    AddDEF(configData, "ScrollCenterOffsetX", 0);
    AddDEF(configData, "ScrollCenterOffsetY", 0);
    AddDEF(configData, "onlineServerList", "");
    AddDEF(configData, "theme", "");
    AddDEF(configData, "disableTransparency", false);

    return configData;
}

DefaultsData* getBrandingDefaults()
{
    DefaultsData* brandingData = new DefaultsData;
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
    AddDEF(brandingData, "font", "fonts/DejaVuSerifCondensed.ttf");
    AddDEF(brandingData, "boldFont", "fonts/DejaVuSerifCondensed-Bold.ttf");
    AddDEF(brandingData, "monoFont", "fonts/dejavusans-mono.ttf");

    return brandingData;
}

DefaultsData* getPathsDefaults()
{
    DefaultsData *pathsData = new DefaultsData;
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
