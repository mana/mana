/*
 *  Gui Skinning
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2009  Aethyra Development Team
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

#include "resources/theme.h"

#include "configuration.h"
#include "log.h"
#include "textrenderer.h"

#include "resources/dye.h"
#include "resources/image.h"
#include "resources/imageset.h"
#include "resources/resourcemanager.h"

#include "utils/filesystem.h"

#include <guichan/font.hpp>
#include <guichan/widget.hpp>

#include <algorithm>
#include <optional>

/**
 * Initializes the directory in which the client looks for GUI themes, which at
 * the same time functions as a fallback directory when looking up files
 * relevant for the GUI theme.
 */
static std::string defaultThemePath;

static void initDefaultThemePath()
{
    defaultThemePath = branding.getStringValue("guiThemePath");

    if (defaultThemePath.empty() || !FS::isDirectory(defaultThemePath))
        defaultThemePath = "graphics/gui/";
}

static std::optional<std::string> findThemePath(const std::string &theme)
{
    if (theme.empty())
        return {};

    std::string themePath = defaultThemePath;
    themePath += theme;

    if (FS::isDirectory(themePath))
        return themePath;

    return {};
}


WidgetState::WidgetState(const gcn::Widget *widget)
    : width(widget->getWidth())
    , height(widget->getHeight())
{
    // x and y are not set based on the widget because the rendering usually
    // happens in local coordinates.

    if (!widget->isEnabled())
        flags |= STATE_DISABLED;
    if (widget->isFocused())
        flags |= STATE_FOCUSED;
}

WidgetState::WidgetState(const gcn::Rectangle &dim, uint8_t flags)
    : x(dim.x)
    , y(dim.y)
    , width(dim.width)
    , height(dim.height)
    , flags(flags)
{}


Skin::~Skin()
{
    // Raw Image* need explicit deletion
    for (auto &state : mStates)
        for (auto &part : state.parts)
            if (auto image = std::get_if<Image *>(&part.data))
                delete *image;
}

void Skin::addState(SkinState state)
{
    mStates.emplace_back(std::move(state));
}

void Skin::draw(Graphics *graphics, const WidgetState &state) const
{
    for (const auto &skinState : mStates)
    {
        if (skinState.stateFlags != (skinState.setFlags & state.flags))
            continue;

        for (const auto &part : skinState.parts)
        {
            std::visit([&](const auto &data) {
                using T = std::decay_t<decltype(data)>;

                if constexpr (std::is_same_v<T, ImageRect>)
                {
                    graphics->drawImageRect(state.x + part.offsetX,
                                            state.y + part.offsetY,
                                            state.width,
                                            state.height,
                                            data);
                }
                else if constexpr (std::is_same_v<T, Image*>)
                {
                    graphics->drawImage(data, state.x + part.offsetX, state.y + part.offsetY);
                }
                else if constexpr (std::is_same_v<T, ColoredRectangle>)
                {
                    graphics->setColor(data.color);
                    graphics->fillRectangle(gcn::Rectangle(state.x + part.offsetX,
                                                           state.y + part.offsetY,
                                                           state.width,
                                                           state.height));
                    graphics->setColor(gcn::Color(255, 255, 255));
                }
            }, part.data);
        }

        break;  // Only draw the first matching state
    }
}

int Skin::getMinWidth() const
{
    int minWidth = 0;

    for (const auto &state : mStates)
    {
        for (const auto &part : state.parts)
        {
            if (auto imageRect = std::get_if<ImageRect>(&part.data))
                minWidth = std::max(minWidth, imageRect->minWidth());
            else if (auto img = std::get_if<Image *>(&part.data))
                minWidth = std::max(minWidth, (*img)->getWidth());
        }
    }

    return minWidth;
}

int Skin::getMinHeight() const
{
    int minHeight = 0;

    for (const auto &state : mStates)
    {
        for (const auto &part : state.parts)
        {
            if (auto imageRect = std::get_if<ImageRect>(&part.data))
                minHeight = std::max(minHeight, imageRect->minHeight());
            else if (auto img = std::get_if<Image *>(&part.data))
                minHeight = std::max(minHeight, (*img)->getHeight());
        }
    }

    return minHeight;
}

void Skin::updateAlpha(float alpha)
{
    for (auto &state : mStates)
    {
        for (auto &part : state.parts)
        {
            if (auto rect = std::get_if<ImageRect>(&part.data))
                rect->setAlpha(alpha);
            else if (auto img = std::get_if<Image *>(&part.data))
                (*img)->setAlpha(alpha);
        }
    }
}


Theme::Theme(const std::string &path)
    : Palette(THEME_COLORS_END)
    , mThemePath(path)
    , mProgressColors(THEME_PROG_END)
{
    listen(Event::ConfigChannel);
    readTheme("theme.xml");

    mColors[HIGHLIGHT].ch = 'H';
    mColors[CHAT].ch = 'C';
    mColors[GM].ch = 'G';
    mColors[PLAYER].ch = 'Y';
    mColors[WHISPER].ch = 'W';
    mColors[IS].ch = 'I';
    mColors[PARTY].ch = 'P';
    mColors[GUILD].ch = 'U';
    mColors[SERVER].ch = 'S';
    mColors[LOGGER].ch = 'L';
    mColors[HYPERLINK].ch = '<';
}

Theme::~Theme() = default;

std::string Theme::prepareThemePath()
{
    initDefaultThemePath();

    // Try theme from settings
    auto themePath = findThemePath(config.theme);

    // Try theme from branding
    if (!themePath)
        themePath = findThemePath(branding.getStringValue("theme"));

    return themePath.value_or(defaultThemePath);
}

std::string Theme::resolvePath(const std::string &path) const
{
    // Need to strip off any dye info for the existence tests
    int pos = path.find('|');
    std::string file;
    if (pos > 0)
        file = path.substr(0, pos);
    else
        file = path;

    // Try the theme
    file = mThemePath + "/" + file;
    if (FS::exists(file))
        return mThemePath + "/" + path;

    // Backup
    return defaultThemePath + "/" + path;
}

ResourceRef<Image> Theme::getImage(const std::string &path) const
{
    return ResourceManager::getInstance()->getImage(resolvePath(path));
}

ResourceRef<Image> Theme::getImageFromTheme(const std::string &path)
{
    return gui->getTheme()->getImage(path);
}

const gcn::Color &Theme::getThemeColor(int type, int alpha)
{
    return gui->getTheme()->getColor(type, alpha);
}

const gcn::Color &Theme::getThemeColor(char c, bool &valid)
{
    return gui->getTheme()->getColor(c, valid);
}

gcn::Color Theme::getProgressColor(int type, float progress)
{
    int color[3] = {0, 0, 0};

    if (const auto &dye = gui->getTheme()->mProgressColors[type])
        dye->getColor(progress, color);

    return gcn::Color(color[0], color[1], color[2]);
}

void Theme::drawSkin(Graphics *graphics, SkinType type, const WidgetState &state) const
{
    getSkin(type).draw(graphics, state);
}

void Theme::drawProgressBar(Graphics *graphics, const gcn::Rectangle &area,
                            const gcn::Color &color, float progress,
                            const std::string &text) const
{
    gcn::Font *oldFont = graphics->getFont();
    gcn::Color oldColor = graphics->getColor();

    WidgetState state;
    state.x = area.x;
    state.y = area.y;
    state.width = area.width;
    state.height = area.height;

    drawSkin(graphics, SkinType::ProgressBar, state);

    // The bar
    if (progress > 0)
    {
        graphics->setColor(color);
        graphics->fillRectangle(gcn::Rectangle(area.x + 4,
                                               area.y + 4,
                                               (int) (progress * (area.width - 8)),
                                               area.height - 8));
    }

    // The label
    if (!text.empty())
    {
        const int textX = area.x + area.width / 2;
        const int textY = area.y + (area.height - boldFont->getHeight()) / 2;

        TextRenderer::renderText(graphics,
                                 text,
                                 textX,
                                 textY,
                                 gcn::Graphics::CENTER,
                                 Theme::getThemeColor(Theme::PROGRESS_BAR),
                                 gui->getFont(),
                                 true,
                                 false);
    }

    graphics->setFont(oldFont);
    graphics->setColor(oldColor);
}

const Skin &Theme::getSkin(SkinType skinType) const
{
    static Skin emptySkin;
    const auto it = mSkins.find(skinType);
    return it != mSkins.end() ? it->second : emptySkin;
}

int Theme::getMinWidth(SkinType skinType) const
{
    return getSkin(skinType).getMinWidth();
}

int Theme::getMinHeight(SkinType skinType) const
{
    return getSkin(skinType).getMinHeight();
}

void Theme::setMinimumOpacity(float minimumOpacity)
{
    if (minimumOpacity > 1.0f)
        return;

    mMinimumOpacity = minimumOpacity;
    updateAlpha();
}

void Theme::updateAlpha()
{
    const float alpha = std::max(config.guiAlpha, mMinimumOpacity);
    if (mAlpha == alpha)
        return;

    mAlpha = alpha;

    for (auto &skin : mSkins)
        skin.second.updateAlpha(mAlpha);
}

void Theme::event(Event::Channel channel, const Event &event)
{
    if (channel == Event::ConfigChannel &&
        event.getType() == Event::ConfigOptionChanged &&
        event.hasValue(&Config::guiAlpha))
    {
        updateAlpha();
    }
}

static bool check(bool value, const char *msg, ...)
{
    if (!value)
    {
        va_list args;
        va_start(args, msg);
        logger->log(msg, args);
        va_end(args);
    }
    return !value;
}

bool Theme::readTheme(const std::string &filename)
{
    logger->log("Loading theme '%s'.", filename.c_str());

    XML::Document doc(resolvePath(filename));
    XML::Node rootNode = doc.rootNode();

    if (!rootNode || rootNode.name() != "theme")
        return false;

    for (auto childNode : rootNode.children())
    {
        if (childNode.name() == "skin")
            readSkinNode(childNode);
        else if (childNode.name() == "color")
            readColorNode(childNode);
        else if (childNode.name() == "progressbar")
            readProgressBarNode(childNode);
    }

    logger->log("Finished loading theme.");

    for (auto &[_, skin] : mSkins)
        skin.updateAlpha(mAlpha);

    return true;
}

static std::optional<SkinType> readSkinType(std::string_view type)
{
    if (type == "Window")               return SkinType::Window;
    if (type == "Popup")                return SkinType::Popup;
    if (type == "SpeechBubble")         return SkinType::SpeechBubble;
    if (type == "Button")               return SkinType::Button;
    if (type == "ButtonUp")             return SkinType::ButtonUp;
    if (type == "ButtonDown")           return SkinType::ButtonDown;
    if (type == "ButtonLeft")           return SkinType::ButtonLeft;
    if (type == "ButtonRight")          return SkinType::ButtonRight;
    if (type == "ButtonClose")          return SkinType::ButtonClose;
    if (type == "ButtonSticky")         return SkinType::ButtonSticky;
    if (type == "CheckBox")             return SkinType::CheckBox;
    if (type == "RadioButton")          return SkinType::RadioButton;
    if (type == "TextField")            return SkinType::TextField;
    if (type == "Tab")                  return SkinType::Tab;
    if (type == "ScrollArea")           return SkinType::ScrollArea;
    if (type == "ScrollAreaHBar")       return SkinType::ScrollAreaHBar;
    if (type == "ScrollAreaHMarker")    return SkinType::ScrollAreaHMarker;
    if (type == "ScrollAreaVBar")       return SkinType::ScrollAreaVBar;
    if (type == "ScrollAreaVMarker")    return SkinType::ScrollAreaVMarker;
    if (type == "DropDownFrame")        return SkinType::DropDownFrame;
    if (type == "DropDownButton")       return SkinType::DropDownButton;
    if (type == "ProgressBar")          return SkinType::ProgressBar;
    if (type == "Slider")               return SkinType::Slider;
    if (type == "SliderHandle")         return SkinType::SliderHandle;
    if (type == "ResizeGrip")           return SkinType::ResizeGrip;
    if (type == "ShortcutBox")          return SkinType::ShortcutBox;
    return {};
}

void Theme::readSkinNode(XML::Node node)
{
    const auto skinTypeStr = node.getProperty("type", std::string());
    const auto skinType = readSkinType(skinTypeStr);
    if (check(skinType.has_value(), "Theme: Unknown skin type '%s'", skinTypeStr.c_str()))
        return;

    auto &skin = mSkins[*skinType];

    node.attribute("frameSize", skin.frameSize);
    node.attribute("padding", skin.padding);
    node.attribute("titleBarHeight", skin.titleBarHeight);
    node.attribute("titleOffsetX", skin.titleOffsetX);
    node.attribute("titleOffsetY", skin.titleOffsetY);

    for (auto childNode : node.children())
        if (childNode.name() == "state")
            readSkinStateNode(childNode, skin);
}

void Theme::readSkinStateNode(XML::Node node, Skin &skin) const
{
    SkinState state;

    auto readFlag = [&] (const char *name, int flag)
    {
        std::optional<bool> value;
        node.attribute(name, value);

        if (value.has_value())
        {
            state.setFlags |= flag;
            state.stateFlags |= *value ? flag : 0;
        }
    };

    readFlag("selected", STATE_SELECTED);
    readFlag("disabled", STATE_DISABLED);
    readFlag("hovered", STATE_HOVERED);
    readFlag("focused", STATE_FOCUSED);

    for (auto childNode : node.children())
    {
        if (childNode.name() == "img")
            readSkinStateImgNode(childNode, state);
        else if (childNode.name() == "rect")
            readSkinStateRectNode(childNode, state);
    }

    skin.addState(std::move(state));
}

template<>
inline void fromString(const char *str, FillMode &value)
{
    if (strcmp(str, "repeat") == 0)
        value = FillMode::Repeat;
    else if (strcmp(str, "stretch") == 0)
        value = FillMode::Stretch;
}

void Theme::readSkinStateImgNode(XML::Node node, SkinState &state) const
{
    const std::string src = node.getProperty("src", std::string());
    if (check(!src.empty(), "Theme: 'img' element has empty 'src' attribute!"))
        return;

    auto image = getImage(src);
    if (check(image, "Theme: Failed to load image '%s'!", src.c_str()))
        return;

    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
    int x = 0;
    int y = 0;
    int width = image->getWidth();
    int height = image->getHeight();

    node.attribute("left", left);
    node.attribute("right", right);
    node.attribute("top", top);
    node.attribute("bottom", bottom);
    node.attribute("x", x);
    node.attribute("y", y);
    node.attribute("width", width);
    node.attribute("height", height);

    if (check(left >= 0 || right >= 0 || top >= 0 || bottom >= 0, "Theme: Invalid border value!"))
        return;
    if (check(x >= 0 || y >= 0, "Theme: Invalid position value!"))
        return;
    if (check(width >= 0 || height >= 0, "Theme: Invalid size value!"))
        return;
    if (check(x + width <= image->getWidth() || y + height <= image->getHeight(), "Theme: Image size out of bounds!"))
        return;

    auto &part = state.parts.emplace_back();

    node.attribute("offsetX", part.offsetX);
    node.attribute("offsetY", part.offsetY);

    if (left + right + top + bottom > 0)
    {
        auto &border = part.data.emplace<ImageRect>();

        node.attribute("fill", border.fillMode);

        const int gridx[4] = {x, x + left, x + width - right, x + width};
        const int gridy[4] = {y, y + top, y + height - bottom, y + height};
        unsigned a = 0;

        for (unsigned y = 0; y < 3; y++)
        {
            for (unsigned x = 0; x < 3; x++)
            {
                border.grid[a] = image->getSubImage(gridx[x],
                                                    gridy[y],
                                                    gridx[x + 1] - gridx[x],
                                                    gridy[y + 1] - gridy[y]);
                a++;
            }
        }
    }
    else
    {
        part.data = image->getSubImage(x, y, width, height);
    }
}

template<>
inline void fromString(const char *str, gcn::Color &value)
{
    if (strlen(str) < 7 || str[0] != '#')
    {
    error:
        logger->log("Error, invalid theme color palette: %s", str);
        value = Palette::BLACK;
        return;
    }

    int v = 0;
    for (int i = 1; i < 7; ++i)
    {
        char c = str[i];
        int n;

        if ('0' <= c && c <= '9')
            n = c - '0';
        else if ('A' <= c && c <= 'F')
            n = c - 'A' + 10;
        else if ('a' <= c && c <= 'f')
            n = c - 'a' + 10;
        else
            goto error;

        v = (v << 4) | n;
    }

    value = gcn::Color(v);
}

void Theme::readSkinStateRectNode(XML::Node node, SkinState &state) const
{
    auto &part = state.parts.emplace_back();
    auto &rect = part.data.emplace<ColoredRectangle>();

    node.attribute("color", rect.color);
    node.attribute("alpha", rect.color.a);
}

static int readColorType(const std::string &type)
{
    static constexpr const char *colors[Theme::THEME_COLORS_END] = {
        "TEXT",
        "SHADOW",
        "OUTLINE",
        "PROGRESS_BAR",
        "BUTTON",
        "BUTTON_DISABLED",
        "TAB",
        "PARTY_CHAT_TAB",
        "PARTY_SOCIAL_TAB",
        "BACKGROUND",
        "HIGHLIGHT",
        "TAB_FLASH",
        "SHOP_WARNING",
        "ITEM_EQUIPPED",
        "CHAT",
        "BUBBLE_TEXT",
        "GM",
        "PLAYER",
        "WHISPER",
        "IS",
        "PARTY",
        "GUILD",
        "SERVER",
        "LOGGER",
        "HYPERLINK",
        "UNKNOWN_ITEM",
        "GENERIC",
        "HEAD",
        "USABLE",
        "TORSO",
        "ONEHAND",
        "LEGS",
        "FEET",
        "TWOHAND",
        "SHIELD",
        "RING",
        "NECKLACE",
        "ARMS",
        "AMMO",
        "SERVER_VERSION_NOT_SUPPORTED"
    };

    if (type.empty())
        return -1;

    for (int i = 0; i < Theme::THEME_COLORS_END; i++)
        if (type == colors[i])
            return i;

    return -1;
}

static Palette::GradientType readColorGradient(const std::string &grad)
{
    static constexpr const char *grads[] = {
        "STATIC",
        "PULSE",
        "SPECTRUM",
        "RAINBOW"
    };

    if (grad.empty())
        return Palette::STATIC;

    for (int i = 0; i < 4; i++)
        if (grad == grads[i])
            return static_cast<Palette::GradientType>(i);

    return Palette::STATIC;
}

void Theme::readColorNode(XML::Node node)
{
    const int type = readColorType(node.getProperty("id", std::string()));
    if (check(type > 0, "Theme: 'color' element has invalid or no 'type' attribute!"))
        return;

    gcn::Color color;
    if (check(node.attribute("color", color), "Theme: 'color' element missing 'color' attribute!"))
        return;

    const GradientType grad = readColorGradient(node.getProperty("effect", std::string()));

    mColors[type].set(type, color, grad, 10);
}

static int readProgressType(const std::string &type)
{
    static constexpr const char *colors[Theme::THEME_PROG_END] = {
        "DEFAULT",
        "HP",
        "MP",
        "NO_MP",
        "EXP",
        "INVY_SLOTS",
        "WEIGHT",
        "JOB"
    };

    if (type.empty())
        return -1;

    for (int i = 0; i < Theme::THEME_PROG_END; i++)
        if (type == colors[i])
            return i;

    return -1;
}

void Theme::readProgressBarNode(XML::Node node)
{
    const int type = readProgressType(node.getProperty("id", std::string()));
    if (type < 0) // invalid or no type given
        return;

    mProgressColors[type] = std::make_unique<DyePalette>(node.getProperty("color", std::string()));
}
