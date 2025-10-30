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

#pragma once

#include "graphics.h"
#include "eventlistener.h"

#include "gui/palette.h"
#include "resources/image.h"
#include "utils/xml.h"

#include <array>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace gcn {
class Widget;
}

class DyePalette;
class Image;
class ImageSet;
class ProgressBar;

class ThemeInfo
{
public:
    ThemeInfo() = default;
    explicit ThemeInfo(const std::string &path);

    bool isValid() const { return !name.empty(); }

    const std::string &getName() const { return name; }
    const std::string &getPath() const { return path; }
    std::string getFullPath() const;
    const XML::Document &getDocument() const { return *doc; }

private:
    std::string name;
    std::string path;
    std::unique_ptr<XML::Document> doc;
};

enum class SkinType
{
    Window,
    ToolWindow,
    Popup,
    SpeechBubble,
    Desktop,
    Button,
    ButtonUp,
    ButtonDown,
    ButtonLeft,
    ButtonRight,
    ButtonClose,
    ButtonSticky,
    CheckBox,
    RadioButton,
    TextField,
    Tab,
    ScrollArea,
    ScrollAreaHBar,
    ScrollAreaHMarker,
    ScrollAreaVBar,
    ScrollAreaVMarker,
    DropDownFrame,
    DropDownButton,
    ProgressBar,
    Slider,
    SliderHandle,
    ResizeGrip,
    ShortcutBox,
    EquipmentBox,
    ItemSlot,
    EmoteSlot,
};

enum StateFlags : uint8_t
{
    STATE_HOVERED   = 0x01,
    STATE_SELECTED  = 0x02,
    STATE_DISABLED  = 0x04,
    STATE_FOCUSED   = 0x08,
};

struct ColoredRectangle
{
    gcn::Color color;
    bool useCurrentColor = false;
    bool filled = true;
};

struct SkinPart
{
    int offsetX = 0;
    int offsetY = 0;
    std::variant<ImageRect, Image *, ColoredRectangle> data;
};

struct TextFormat
{
    bool bold = false;
    gcn::Color color;
    std::optional<gcn::Color> outlineColor;
    std::optional<gcn::Color> shadowColor;
};

struct SkinState
{
    uint8_t stateFlags  = 0;
    uint8_t setFlags = 0;
    TextFormat textFormat;
    std::vector<SkinPart> parts;
};

struct WidgetState
{
    WidgetState() = default;
    explicit WidgetState(const gcn::Widget *widget);
    explicit WidgetState(const gcn::Rectangle &dim, uint8_t flags = 0);

    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    uint8_t flags = 0;
};

class Skin
{
    public:
        Skin() = default;
        ~Skin();

        void addState(SkinState state);

        void draw(Graphics *graphics, const WidgetState &state) const;

        const SkinState *getState(uint8_t flags) const;

        /**
         * Returns the minimum width which can be used with this skin.
         */
        int getMinWidth() const;

        /**
         * Returns the minimum height which can be used with this skin.
         */
        int getMinHeight() const;

        /**
         * Updates the alpha value of the skin
         */
        void updateAlpha(float alpha);

        int width = 0;
        int height = 0;
        int frameSize = 0;
        int padding = 0;
        int spacing = 0;
        int titleBarHeight = 0;
        int titleOffsetX = 0;
        int titleOffsetY = 0;
        int palette = 0;
        bool showButtons = true;

    private:
        std::vector<SkinState> mStates;
};

class Theme : public EventListener
{
    public:
        static std::string prepareThemePath();
        static std::vector<ThemeInfo> getAvailableThemes();

        Theme(const ThemeInfo &themeInfo);
        ~Theme() override;

        const std::string &getThemePath() const { return mThemePath; }

        /**
         * Returns the patch to the given GUI resource relative to the theme
         * or, if it isn't in the theme, relative to 'graphics/gui'.
         */
        std::string resolvePath(const std::string &path) const;
        static ResourceRef<Image> getImageFromTheme(const std::string &path);

        enum ThemePalette {
            TEXT,
            BLACK,          // Color 0
            RED,            // Color 1
            GREEN,          // Color 2
            BLUE,           // Color 3
            ORANGE,         // Color 4
            YELLOW,         // Color 5
            PINK,           // Color 6
            PURPLE,         // Color 7
            GRAY,           // Color 8
            BROWN,          // Color 9
            CARET,
            SHADOW,
            OUTLINE,
            PARTY_TAB,
            WHISPER_TAB,
            BACKGROUND,
            HIGHLIGHT,
            HIGHLIGHT_TEXT,
            TAB_FLASH,
            SHOP_WARNING,
            ITEM_EQUIPPED,
            CHAT,
            OLDCHAT,
            AWAYCHAT,
            BUBBLE_TEXT,
            GM,
            GLOBAL,
            PLAYER,
            WHISPER,
            IS,
            PARTY,
            GUILD,
            SERVER,
            LOGGER,
            HYPERLINK,
            UNKNOWN_ITEM,
            GENERIC,
            HEAD,
            USABLE,
            TORSO,
            ONEHAND,
            LEGS,
            FEET,
            TWOHAND,
            SHIELD,
            RING,
            NECKLACE,
            ARMS,
            AMMO,
            SERVER_VERSION_NOT_SUPPORTED,
            THEME_COLORS_END
        };

        enum ProgressPalette {
            PROG_DEFAULT,
            PROG_HP,
            PROG_MP,
            PROG_NO_MP,
            PROG_EXP,
            PROG_INVY_SLOTS,
            PROG_WEIGHT,
            PROG_JOB,
            THEME_PROG_END
        };

        /**
         * Gets the color associated with the type in the default palette (0).
         *
         * @param type the color type requested
         * @return the requested color
         */
        static const gcn::Color &getThemeColor(int type);

        static gcn::Color getProgressColor(int type, float progress);

        const Palette &getPalette(size_t index) const;

        /**
         * Returns a color from the default palette (0).
         */
        const gcn::Color &getColor(int type) const;

        /**
         * Returns the color ID associated with a character, if it exists.
         * Returns no value if the character is not found.
         *
         * @param c character requested
         * @return the requested color or none
         */
        static std::optional<int> getColorIdForChar(char c);

        void drawSkin(Graphics *graphics, SkinType type, const WidgetState &state) const;
        void drawProgressBar(Graphics *graphics,
                             const gcn::Rectangle &area,
                             const gcn::Color &color,
                             float progress,
                             const std::string &text = std::string(),
                             ProgressPalette progressType = ProgressPalette::THEME_PROG_END) const;

        const Skin &getSkin(SkinType skinType) const;

        const Image *getIcon(const std::string &name) const;

        /**
         * Get the current GUI alpha value.
         */
        int getGuiAlpha() const { return static_cast<int>(mAlpha * 255.0f); }

        /**
         * Get the minimum opacity allowed to skins.
         */
        float getMinimumOpacity() const { return mMinimumOpacity; }

        /**
         * Set the minimum opacity allowed to skins.
         * Set a negative value to free the minimum allowed.
         */
        void setMinimumOpacity(float minimumOpacity);

        void event(Event::Channel channel, const Event &event) override;

    private:
        /**
         * Updates the alpha values of all of the skins and images.
         */
        void updateAlpha();

        ResourceRef<Image> getImage(const std::string &path) const;

        bool readTheme(const ThemeInfo &themeInfo);
        void readSkinNode(XML::Node node);
        void readSkinStateNode(XML::Node node, Skin &skin) const;
        void readSkinStateImgNode(XML::Node node, SkinState &state) const;
        void readIconNode(XML::Node node);
        void readPaletteNode(XML::Node node);
        void readProgressBarNode(XML::Node node);

        std::string mThemePath;
        std::map<SkinType, Skin> mSkins;
        std::map<std::string, Image *> mIcons;

        /**
         * Tells if the current skins opacity
         * should not get less than the given value
         */
        float mMinimumOpacity = 0.0f;
        float mAlpha = 1.0;

        std::vector<Palette> mPalettes;
        std::array<std::unique_ptr<DyePalette>, THEME_PROG_END> mProgressColors;
        std::array<std::optional<TextFormat>, THEME_PROG_END> mProgressTextFormats;
};
