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

#include "configuration.h"

#include "log.h"

#include "utils/stringutils.h"

template<typename T>
struct Option
{
    Option(const char *name, const T &value, const T &defaultValue)
        : name(name), value(value), defaultValue(defaultValue)
    {}

    const char *name;
    const T &value;
    const T &defaultValue;
};

template<typename T>
static void serialize(XML::Writer &writer, const Option<T> &option)
{
    if (option.value == option.defaultValue)
        return;

    writer.startElement("option");
    writer.addAttribute("name", option.name);
    writer.addAttribute("value", option.value);
    writer.endElement();
}

static void serialize(XML::Writer &writer, const ItemShortcutEntry &itemShortcut)
{
    writer.startElement("itemshortcut");
    writer.addAttribute("index", itemShortcut.index);
    writer.addAttribute("id", itemShortcut.itemId);
    writer.endElement();
}

static void serialize(XML::Writer &writer, const EmoteShortcutEntry &emoteShortcut)
{
    writer.startElement("emoteshortcut");
    writer.addAttribute("index", emoteShortcut.index);
    writer.addAttribute("id", emoteShortcut.emoteId);
    writer.endElement();
}

static void serialize(XML::Writer &writer, const Outfit &outfit)
{
    writer.startElement("outfit");
    writer.addAttribute("index", outfit.index);
    writer.addAttribute("items", outfit.items);
    writer.addAttribute("unequip", outfit.unequip);
    writer.endElement();
}

static void serialize(XML::Writer &writer, const UserColor &color)
{
    if (!color.color.empty())
        writer.addAttribute("color", color.color);

    writer.addAttribute("gradient", color.gradient);

    if (color.delay)
        writer.addAttribute("delay", *color.delay);
}

static void serialize(XML::Writer &writer, const WindowState &state)
{
    if (state.x)        writer.addAttribute("x",        *state.x);
    if (state.y)        writer.addAttribute("y",        *state.y);
    if (state.width)    writer.addAttribute("width",    *state.width);
    if (state.height)   writer.addAttribute("height",   *state.height);
    if (state.visible)  writer.addAttribute("visible",  *state.visible);
    if (state.sticky)   writer.addAttribute("sticky",   *state.sticky);
}

static const char *serverTypeToString(ServerType type)
{
    switch (type)
    {
    case ServerType::TmwAthena:
        return "tmwathena";
    case ServerType::ManaServ:
        return "manaserv";
    default:
        return "";
    }
}

static void serialize(XML::Writer &writer, const ServerInfo &server)
{
    writer.startElement("server");

    writer.addAttribute("name", server.name);
    writer.addAttribute("type", serverTypeToString(server.type));

    writer.startElement("connection");
    writer.addAttribute("hostname", server.hostname);
    writer.addAttribute("port", server.port);
    writer.endElement(); // connection

    if (!server.description.empty())
    {
        writer.startElement("description");
        writer.writeText(server.description);
        writer.endElement();
    }

    if (!server.persistentIp)
    {
        writer.startElement("persistentIp");
        writer.writeText(server.persistentIp ? "1" : "0");
        writer.endElement();
    }

    writer.endElement(); // server
}

/**
 * Reads options into a statically typed settings struct, erasing each option
 * that it recognizes. Whatever remains was not recognized.
 */
template<typename T>
class OptionDeserializer
{
public:
    OptionDeserializer(std::map<std::string, std::string> &options, T &target)
        : mOptions(options)
        , mTarget(target)
    {}

    template<typename M>
    void operator()(const char *name, M T::*member) const
    {
        auto it = mOptions.find(name);
        if (it == mOptions.end())
            return;

        fromString(it->second.data(), mTarget.*member);
        mOptions.erase(it);
    }

private:
    std::map<std::string, std::string> &mOptions;
    T &mTarget;
};

template<typename T>
void serdeOptions(T option)
{
    option("OverlayDetail",                 &Config::overlayDetail);
    option("speechBubblecolor",             &Config::speechBubblecolor);
    option("speechBubbleAlpha",             &Config::speechBubbleAlpha);
    option("speech",                        &Config::speech);
    option("visiblenames",                  &Config::visibleNames);
    option("showgender",                    &Config::showGender);
    option("showMonstersTakedDamage",       &Config::showMonstersTakedDamage);
    option("showWarps",                     &Config::showWarps);
    option("hideCompletedQuests",           &Config::hideCompletedQuests);
    option("particleMaxCount",              &Config::particleMaxCount);
    option("particleFastPhysics",           &Config::particleFastPhysics);
    option("particleEmitterSkip",           &Config::particleEmitterSkip);
    option("particleeffects",               &Config::particleEffects);
    option("logToStandardOut",              &Config::logToStandardOut);
    option("opengl",                        &Config::opengl);
    option("vsync",                         &Config::vsync);
    option("reduceInputLag",                &Config::reduceInputLag);
    option("windowmode",                    &Config::windowMode);
    option("screenwidth",                   &Config::screenWidth);
    option("screenheight",                  &Config::screenHeight);
    option("scale",                         &Config::scale);
    option("sound",                         &Config::sound);
    option("sfxVolume",                     &Config::sfxVolume);
    option("notificationsVolume",           &Config::notificationsVolume);
    option("musicVolume",                   &Config::musicVolume);
    option("fpslimit",                      &Config::fpsLimit);

    option("remember",                      &Config::remember);
    option("username",                      &Config::username);
    option("lastCharacter",                 &Config::lastCharacter);
    option("updatehost",                    &Config::updatehost);
    option("screenshotDirectory",           &Config::screenshotDirectory);
    option("screenshotDirectorySuffix",     &Config::screenshotDirectorySuffix);
    option("useScreenshotDirectorySuffix",  &Config::useScreenshotDirectorySuffix);

    option("EnableSync",                    &Config::enableSync);

    option("joystickEnabled",               &Config::joystickEnabled);
    option("upTolerance",                   &Config::upTolerance);
    option("downTolerance",                 &Config::downTolerance);
    option("leftTolerance",                 &Config::leftTolerance);
    option("rightTolerance",                &Config::rightTolerance);

    option("logNpcInGui",                   &Config::logNpcInGui);
    option("download-music",                &Config::downloadMusic);
    option("guialpha",                      &Config::guiAlpha);
    option("ChatLogLength",                 &Config::chatLogLength);
    option("enableChatLog",                 &Config::enableChatLog);
    option("whispertab",                    &Config::whisperTab);
    option("customcursor",                  &Config::customCursor);
    option("showownname",                   &Config::showOwnName);
    option("showpickupparticle",            &Config::showPickupParticle);
    option("showpickupchat",                &Config::showPickupChat);
    option("showMinimap",                   &Config::showMinimap);
    option("fontSize",                      &Config::fontSize);
    option("ReturnToggles",                 &Config::returnTogglesChat);
    option("ScrollLaziness",                &Config::scrollLaziness);
    option("ScrollRadius",                  &Config::scrollRadius);
    option("ScrollCenterOffsetX",           &Config::scrollCenterOffsetX);
    option("ScrollCenterOffsetY",           &Config::scrollCenterOffsetY);
    option("onlineServerList",              &Config::onlineServerList);
    option("theme",                         &Config::theme);
    option("disableTransparency",           &Config::disableTransparency);

    option("persistent-player-list",        &Config::persistentPlayerList);
    option("player-ignore-strategy",        &Config::playerIgnoreStrategy);
    option("default-player-permissions",    &Config::defaultPlayerPermissions);
}

void serialize(XML::Writer &writer, const Config &config)
{
    const Config defaults;
    auto serializeOption = [&](const char *name, auto member) {
        serialize(writer, Option { name, config.*member, defaults.*member });
    };

    writer.startElement("configuration");

    serdeOptions(serializeOption);

    for (const auto &[name, value] : config.unknownOptions)
        serialize(writer, Option { name.c_str(), value, std::string() });

    for (const auto &[action, key] : config.keys)
    {
        writer.startElement("key");
        writer.addAttribute("action", action);
        writer.addAttribute("key", key);
        writer.endElement();
    }

    for (auto &itemShortcut : config.itemShortcuts)
        serialize(writer, itemShortcut);

    for (auto &emoteShortcut : config.emoteShortcuts)
        serialize(writer, emoteShortcut);

    for (auto &outfit : config.outfits)
        serialize(writer, outfit);

    for (auto &[type, color] : config.colors)
    {
        writer.startElement("color");
        writer.addAttribute("type", type);

        serialize(writer, color);

        writer.endElement();
    }

    for (const auto &[name, state] : config.windows)
    {
        writer.startElement("window");
        writer.addAttribute("name", name);

        serialize(writer, state);

        writer.endElement(); // window
    }

    for (const auto &server : config.servers)
    {
        if (server.save && server.isValid())
            serialize(writer, server);
    }

    for (const auto &[name, relation] : config.players)
    {
        writer.startElement("player");
        writer.addAttribute("name", name);
        writer.addAttribute("relation", static_cast<int>(relation));
        writer.endElement();
    }

    writer.endElement(); // configuration
}

void deserialize(XML::Node node, ItemShortcutEntry &itemShortcut)
{
    node.attribute("index", itemShortcut.index);
    node.attribute("id", itemShortcut.itemId);
}

void deserialize(XML::Node node, EmoteShortcutEntry &emoteShortcut)
{
    node.attribute("index", emoteShortcut.index);
    node.attribute("id", emoteShortcut.emoteId);
}

void deserialize(XML::Node node, Outfit &outfit)
{
    node.attribute("index", outfit.index);
    node.attribute("items", outfit.items);
    node.attribute("unequip", outfit.unequip);
}

void deserialize(XML::Node node, UserColor &color)
{
    node.attribute("color", color.color);
    node.attribute("gradient", color.gradient);
    node.attribute("delay", color.delay);
}

void deserialize(XML::Node node, WindowState &state)
{
    node.attribute("x", state.x);
    node.attribute("y", state.y);
    node.attribute("width", state.width);
    node.attribute("height", state.height);
    node.attribute("visible", state.visible);
    node.attribute("sticky", state.sticky);
}

void deserialize(XML::Node node, ServerInfo &server)
{
    node.attribute("name", server.name);

    std::string type;
    node.attribute("type", type);
    server.type = ServerInfo::parseType(type);
    server.save = true;

    for (auto node : node.children()) {
        if (node.name() == "connection") {
            node.attribute("hostname", server.hostname);
            node.attribute("port", server.port);
        } else if (node.name() == "description") {
            server.description = node.textContent();
        } else if (node.name() == "persistentIp") {
            const std::string value { node.textContent() };
            server.persistentIp = getBoolFromString(value, server.persistentIp);
        }
    }
}

void deserialize(XML::Node node, Config &config)
{
    std::map<std::string, std::string> options;

    for (auto node : node.children()) {
        if (node.name() == "option") {
            std::string name;
            if (!node.attribute("name", name))
                continue;
            node.attribute("value", options[name]);
        } else if (node.name() == "list") {
            // Backwards compatibility for old configuration files
            for (auto node : node.children()) {
                if (node.name() == "player") {
                    std::string playerName;
                    PlayerRelation relation = PlayerRelation::Neutral;

                    for (auto node : node.children()) {
                        if (node.name() == "option") {
                            std::string optionName;

                            if (node.attribute("name", optionName)) {
                                if (optionName == "name")
                                    node.attribute("value", playerName);
                                else if (optionName == "relation")
                                    node.attribute("value", relation);
                            }
                        }
                    }

                    if (!playerName.empty())
                        config.players[playerName] = relation;
                }
            }
        } else if (node.name() == "key") {
            std::string action;
            node.attribute("action", action);
            if (!action.empty())
                node.attribute("key", config.keys[action]);
        } else if (node.name() == "itemshortcut") {
            deserialize(node, config.itemShortcuts.emplace_back());
        } else if (node.name() == "emoteshortcut") {
            deserialize(node, config.emoteShortcuts.emplace_back());
        } else if (node.name() == "outfit") {
            deserialize(node, config.outfits.emplace_back());
        } else if (node.name() == "color") {
            std::string type;
            node.attribute("type", type);
            deserialize(node, config.colors[type]);
        } else if (node.name() == "window") {
            std::string name;
            node.attribute("name", name);
            deserialize(node, config.windows[name]);
        } else if (node.name() == "player") {
            std::string name;
            node.attribute("name", name);
            if (!name.empty())
                node.attribute("relation", config.players[name]);
        } else if (node.name() == "server") {
            deserialize(node, config.servers.emplace_back());
        }
    }

    serdeOptions(OptionDeserializer(options, config));

    config.unknownOptions = std::move(options);
}

std::map<std::string, std::string> readOptions(XML::Node node)
{
    std::map<std::string, std::string> options;

    if (!node)
        return options;

    for (auto child : node.children()) {
        if (child.name() != "option")
            continue;

        std::string name;
        if (child.attribute("name", name))
            child.attribute("value", options[name]);
    }

    return options;
}

void warnUnknownOptions(const std::string &fileName,
                        const std::map<std::string, std::string> &options)
{
    for (const auto &[name, value] : options)
        Log::warn("Unknown option \"%s\" in %s", name.c_str(), fileName.c_str());
}

void deserialize(std::map<std::string, std::string> &options, Branding &branding)
{
    OptionDeserializer deserializeOption(options, branding);

    deserializeOption("wallpapersPath", &Branding::wallpapersPath);
    deserializeOption("wallpaperFile", &Branding::wallpaperFile);
    deserializeOption("appName", &Branding::appName);
    deserializeOption("appIcon", &Branding::appIcon);
    deserializeOption("loginMusic", &Branding::loginMusic);
    deserializeOption("defaultServer", &Branding::defaultServer);
    deserializeOption("defaultPort", &Branding::defaultPort);
    deserializeOption("defaultServerType", &Branding::defaultServerType);
    deserializeOption("appShort", &Branding::appShort);
    deserializeOption("defaultUpdateHost", &Branding::defaultUpdateHost);
    deserializeOption("helpPath", &Branding::helpPath);
    deserializeOption("onlineServerList", &Branding::onlineServerList);
    deserializeOption("guiThemePath", &Branding::guiThemePath);
    deserializeOption("theme", &Branding::theme);
    deserializeOption("font", &Branding::font);
    deserializeOption("boldFont", &Branding::boldFont);
    deserializeOption("monoFont", &Branding::monoFont);
}

void deserialize(std::map<std::string, std::string> &options, Paths &paths)
{
    OptionDeserializer deserializeOption(options, paths);

    deserializeOption("itemIcons", &Paths::itemIcons);
    deserializeOption("unknownItemFile", &Paths::unknownItemFile);
    deserializeOption("sprites", &Paths::sprites);
    deserializeOption("spriteErrorFile", &Paths::spriteErrorFile);

    deserializeOption("particles", &Paths::particles);
    deserializeOption("levelUpEffectFile", &Paths::levelUpEffectFile);
    deserializeOption("portalEffectFile", &Paths::portalEffectFile);
    deserializeOption("hitEffectId", &Paths::hitEffectId);
    deserializeOption("criticalHitEffectId", &Paths::criticalHitEffectId);
    deserializeOption("newQuestEffectId", &Paths::newQuestEffectId);
    deserializeOption("completeQuestEffectId", &Paths::completeQuestEffectId);

    // This makes sure that actors positioned on the center of a tile have
    // their sprite aligned to the bottom of that tile. The default maintains
    // compatibility with existing sprites.
    deserializeOption("spriteOffsetY", &Paths::spriteOffsetY);

    deserializeOption("minimaps", &Paths::minimaps);
    deserializeOption("maps", &Paths::maps);

    deserializeOption("sfx", &Paths::sfx);
    deserializeOption("attackSfxFile", &Paths::attackSfxFile);
    deserializeOption("music", &Paths::music);

    deserializeOption("wallpapers", &Paths::wallpapers);
    deserializeOption("wallpaperFile", &Paths::wallpaperFile);

    deserializeOption("help", &Paths::help);
}

void deserialize(std::map<std::string, std::string> &options, Portable &portable)
{
    OptionDeserializer deserializeOption(options, portable);

    deserializeOption("dataDir", &Portable::dataDir);
    deserializeOption("configDir", &Portable::configDir);
    deserializeOption("screenshotDir", &Portable::screenshotDir);
}
