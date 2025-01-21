/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2024  The Mana Developers
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
#include "defaults.h"
#include "playerrelations.h"
#include "video.h"

#include "utils/xml.h"

#include "net/serverinfo.h"

#include <cassert>
#include <map>
#include <optional>
#include <string>
#include <vector>

/**
 * Configuration object, mapping values to names and possibly containing
 * lists of further configuration objects
 *
 * \ingroup CORE
 */
class ConfigurationObject
{
    friend class Configuration;

    public:
        virtual ~ConfigurationObject();

        /**
         * Sets an option using a string value.
         *
         * \param key Option identifier.
         * \param value Value.
         */
        void setValue(const std::string &key, const std::string &value);

        /**
         * Gets a value as string.
         *
         * \param key Option identifier.
         * \param deflt Default option if not there or error.
         */
        std::string getValue(const std::string &key,
                             const std::string &deflt) const;

        int getValue(const std::string &key, int deflt) const;

        unsigned getValue(const std::string &key, unsigned deflt) const;

        double getValue(const std::string &key, double deflt) const;

        /**
         * Re-sets all data in the configuration
         */
        void clear();

    protected:
        void initFromXML(XML::Node node);

        std::map<std::string, std::string> mOptions;
};

/**
 * Configuration handler for reading (and writing).
 *
 * \ingroup CORE
 */
class Configuration : public ConfigurationObject
{
    public:
        Configuration() = default;

        ~Configuration() override;

        /**
         * Reads config file and parse all options into memory.
         *
         * @param filename path to config file
         * @param useResManager Make use of the resource manager.
         */
        void init(const std::string &filename, bool useResManager = false);

        /**
         * Set the default values for each keys.
         *
         * @param defaultsData data used as defaults.
         */
        void setDefaultValues(DefaultsData *defaultsData);

        /**
         * returns a value corresponding to the given key.
         * The default value returned in based on fallbacks registry.
         * @see defaults.h
         */
        int getIntValue(const std::string &key) const;

        float getFloatValue(const std::string &key) const;

        std::string getStringValue(const std::string &key) const;

        bool getBoolValue(const std::string &key) const;

        VariableData *getDefault(const std::string &key,
                                 VariableData::DataType type) const;
    private:
        /**
         * Clean up the default values member.
         */
        void cleanDefaults();

        std::string mConfigPath;       /**< Location of config file */
        DefaultsData *mDefaultsData = nullptr;   /**< Defaults of value for a given key */
};

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
    int particleMaxCount = 3000;
    int particleFastPhysics = 0;
    int particleEmitterSkip = 1;
    bool particleEffects = true;
    bool logToStandardOut = false;
    bool opengl = false;
    bool vsync = true;
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

    bool enableSync = false;

    bool joystickEnabled = false;
    int upTolerance = 100;
    int downTolerance = 100;
    int leftTolerance = 100;
    int rightTolerance = 100;

    bool logNpcInGui = true;
    bool downloadMusic = false;
    float guiAlpha = 0.8f;
    int chatLogLength = 128;
    bool enableChatLog = false;
    bool whisperTab = true;
    bool customCursor = true;
    bool showOwnName = false;
    bool showPickupParticle = true;
    bool showPickupChat = false;
    bool showMinimap = true;
    int fontSize = 12;
    bool returnTogglesChat = false;
    int scrollLaziness = 16;
    int scrollRadius = 0;
    int scrollCenterOffsetX = 0;
    int scrollCenterOffsetY = 0;
    std::string onlineServerList;
    std::string theme;
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

void serialize(XML::Writer &writer, const Config &config);
void deserialize(XML::Node node, Config &config);

extern Config config;
extern Configuration branding;
extern Configuration paths;

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
