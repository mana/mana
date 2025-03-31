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

#include <map>
#include <memory>
#include <string>
#include <variant>

namespace gcn {
class Widget;
}

class DyePalette;
class Image;
class ImageSet;
class ProgressBar;

enum class SkinType
{
    Window,
    Popup,
    SpeechBubble,
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
};

enum StateFlags : uint8_t
{
    STATE_NORMAL    = 0x01,
    STATE_HOVERED   = 0x02,
    STATE_SELECTED  = 0x04,
    STATE_DISABLED  = 0x08,
    STATE_FOCUSED   = 0x10,
};

struct ColoredRectangle
{
    gcn::Color color;
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

        int frameSize = 0;
        int padding = 0;
        int titleBarHeight = 0;
        int titleOffsetX = 0;
        int titleOffsetY = 0;

    private:
        std::vector<SkinState> mStates;
};

class Theme : public Palette, public EventListener
{
    public:
        static std::string prepareThemePath();

        Theme(const std::string &path);
        ~Theme() override;

        /**
         * Returns the patch to the given GUI resource relative to the theme
         * or, if it isn't in the theme, relative to 'graphics/gui'.
         */
        std::string resolvePath(const std::string &path) const;
        static ResourceRef<Image> getImageFromTheme(const std::string &path);

        enum ThemePalette {
            TEXT,
            SHADOW,
            OUTLINE,
            PARTY_CHAT_TAB,
            PARTY_SOCIAL_TAB,
            BACKGROUND,
            HIGHLIGHT,
            TAB_FLASH,
            SHOP_WARNING,
            ITEM_EQUIPPED,
            CHAT,
            BUBBLE_TEXT,
            GM,
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
         * Gets the color associated with the type. Sets the alpha channel
         * before returning.
         *
         * @param type the color type requested
         * @param alpha alpha channel to use
         *
         * @return the requested color
         */
        static const gcn::Color &getThemeColor(int type, int alpha = 255);
        static const gcn::Color &getThemeColor(char c, bool &valid);

        static gcn::Color getProgressColor(int type, float progress);

        void drawSkin(Graphics *graphics, SkinType type, const WidgetState &state) const;
        void drawProgressBar(Graphics *graphics,
                             const gcn::Rectangle &area,
                             const gcn::Color &color,
                             float progress,
                             const std::string &text = std::string()) const;

        const Skin &getSkin(SkinType skinType) const;

        int getMinWidth(SkinType skinType) const;
        int getMinHeight(SkinType skinType) const;

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

        bool readTheme(const std::string &filename);
        void readSkinNode(XML::Node node);
        void readSkinStateNode(XML::Node node, Skin &skin) const;
        void readSkinStateTextNode(XML::Node node, SkinState &state) const;
        void readSkinStateImgNode(XML::Node node, SkinState &state) const;
        void readSkinStateRectNode(XML::Node node, SkinState &state) const;
        void readColorNode(XML::Node node);
        void readProgressBarNode(XML::Node node);

        std::string mThemePath;
        std::map<SkinType, Skin> mSkins;

        /**
         * Tells if the current skins opacity
         * should not get less than the given value
         */
        float mMinimumOpacity = 0.0f;
        float mAlpha = 1.0;

        std::vector<std::unique_ptr<DyePalette>> mProgressColors;
};
