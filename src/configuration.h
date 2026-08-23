/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#pragma once

#include "being.h"
#include "playerrelations.h"
#include "video.h"

#include "utils/xml.h"

#include "net/serverinfo.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

struct ItemShortcutEntry
{
    int index;
    int itemId;
};

struct EmoteShortcutEntry
{
    int index;
    int emoteId;
};

struct Outfit
{
    int index;
    std::string items;
    bool unequip;
};

struct UserColor
{
    std::string color;
    int gradient;
    std::optional<int> delay;
};

struct WindowState
{
    std::optional<int> x;
    std::optional<int> y;
    std::optional<int> width;
    std::optional<int> height;
    std::optional<bool> visible;
    std::optional<bool> sticky;
};

/**
 * Client configuration.
 */
struct Config
{
    int overlayDetail = 2;
    std::string speechBubblecolor = "000000";
    float speechBubbleAlpha = 1.0f;
    Being::Speech speech = Being::TEXT_OVERHEAD;
    bool visibleNames = true;
    bool showGender = false;
    bool showMonstersTakedDamage = false;
    bool showWarps = true;
    bool hideCompletedQuests = false;
    int particleMaxCount = 3000;
    int particleFastPhysics = 0;
    int particleEmitterSkip = 1;
    bool particleEffects = true;
    bool logToStandardOut = false;
    bool opengl = false;
    bool vsync = true;
    bool reduceInputLag = true;
    WindowMode windowMode = WindowMode::Windowed;
    int screenWidth = defaultScreenWidth;
    int screenHeight = defaultScreenHeight;
    int scale = 0;
    bool sound = true;
    int sfxVolume = 100;
    int notificationsVolume = 100;
    int musicVolume = 60;
    int fpsLimit = 0;

    bool remember = true;
    std::string username;
    std::string lastCharacter;
    std::string updatehost;
    std::string afkMessage;
    std::string screenshotDirectory;
    std::string screenshotDirectorySuffix;
    bool useScreenshotDirectorySuffix = true;

    bool enableSync = false;    // Should we honor server "Stop Walking" packets

    bool joystickEnabled = false;
    int upTolerance = 100;
    int downTolerance = 100;
    int leftTolerance = 100;
    int rightTolerance = 100;

    bool logNpcInGui = true;
    bool downloadMusic = true;
    float guiAlpha = 0.9f;
    int chatLogLength = 256;
    bool enableChatLog = false;
    bool whisperTab = true;
    bool customCursor = true;
    bool showOwnName = false;
    bool showPickupParticle = true;
    bool showPickupChat = true;
    int pickupRange = 1;
    bool showMinimap = true;
    int fontSize = 12;
    bool returnTogglesChat = false;
    int scrollLaziness = 16;
    int scrollRadius = 0;
    int scrollCenterOffsetX = 0;
    int scrollCenterOffsetY = 0;
    std::string onlineServerList;
    std::string theme = "jewelry";
    std::string language;   // empty means the system language is used
    bool disableTransparency = false;

    bool persistentPlayerList = true;
    std::string playerIgnoreStrategy = DEFAULT_IGNORE_STRATEGY;
    unsigned defaultPlayerPermissions = PlayerPermissions::DEFAULT;

    std::map<std::string, std::string> keys;
    std::vector<ItemShortcutEntry> itemShortcuts;
    std::vector<EmoteShortcutEntry> emoteShortcuts;
    std::vector<Outfit> outfits;
    std::map<std::string, UserColor> colors;
    std::map<std::string, WindowState> windows;
    std::map<std::string, PlayerRelation> players;
    ServerInfos servers;

    // For compatibility with old configuration files or different clients
    std::map<std::string, std::string> unknownOptions;
};

/**
 * Branding configuration.
 */
struct Branding
{
    std::string wallpapersPath;
    std::string wallpaperFile;
    std::string appName;
    std::string appIcon = "icons/mana";
    std::string loginMusic = "system/Magick - Real.ogg";
    std::string defaultServer;
    std::optional<uint16_t> defaultPort;
    std::string defaultServerType = "tmwathena";
    std::string appShort;
    std::string defaultUpdateHost;
    std::string helpPath;
    std::string onlineServerList;
    std::string guiThemePath;
    std::string theme;
    std::string font = "fonts/dejavusans.ttf";
    std::string boldFont = "fonts/dejavusans-bold.ttf";
    std::string monoFont = "fonts/dejavusans-mono.ttf";

    /**
     * The application name. A branding file that leaves it empty is treated
     * the same as one that does not set it at all.
     */
    std::string name() const { return appName.empty() ? "Mana" : appName; }

    /**
     * The short application name, which determines the settings directory.
     */
    std::string shortName() const { return appShort.empty() ? "mana" : appShort; }
};

/**
 * General game data configuration.
 */
struct Paths
{
    std::string itemIcons = "graphics/items/";
    std::string unknownItemFile = "unknown-item.png";
    std::string sprites = "graphics/sprites/";
    std::string spriteErrorFile = "error.xml";

    std::string particles = "graphics/particles/";
    std::string levelUpEffectFile = "levelup.particle.xml";
    std::string portalEffectFile = "warparea.particle.xml";
    int hitEffectId = 26;
    int criticalHitEffectId = 28;
    int newQuestEffectId = -1;
    int completeQuestEffectId = -1;

    // This is makes sure that actors positioned on the center of a tile have
    // their sprite aligned to the bottom of that tile. The default maintains
    // compatibility with existing sprites.
    int spriteOffsetY = 16;

    std::string minimaps = "graphics/minimaps/";
    std::string maps = "maps/";

    std::string sfx = "sfx/";
    std::string attackSfxFile = "fist-swish.ogg";
    std::string music = "music/";

    std::string wallpapers = "graphics/images/";
    std::string wallpaperFile = "login_wallpaper.png";

    std::string help = "help/";
};

/**
 * Portable installation settings, which point at directories relative to the
 * one the client was installed in.
 */
struct Portable
{
    std::string dataDir;
    std::string configDir;
    std::string screenshotDir;
};

/**
 * Reads the "option" elements below the given node into a map.
 */
std::map<std::string, std::string> readOptions(XML::Node node);

/**
 * Logs a warning for each of the given options, which are the ones that were
 * not recognized while reading the given file.
 */
void warnUnknownOptions(const std::string &fileName,
                        const std::map<std::string, std::string> &options);

void serialize(XML::Writer &writer, const Config &config);
void deserialize(XML::Node node, Config &config);
void deserialize(std::map<std::string, std::string> &options, Branding &branding);
void deserialize(std::map<std::string, std::string> &options, Paths &paths);
void deserialize(std::map<std::string, std::string> &options, Portable &portable);

extern Config config;
extern Branding branding;
extern Paths paths;

/**
 * Sets the given Config member and sends a change event.
 */
template<typename T>
void setConfigValue(T Config::*member, const T &value)
{
    if (config.*member == value)
        return;

    config.*member = value;
    Event(Event::ConfigOptionChanged, member).trigger(Event::ConfigChannel);
}
