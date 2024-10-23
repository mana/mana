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

#include <map>
#include <memory>
#include <string>

class DyePalette;
class Image;
class ImageSet;
class ProgressBar;

class Skin
{
    public:
        Skin(ImageRect skin, Image *close, Image *stickyUp, Image *stickyDown);

        ~Skin();

        /**
         * Returns the background skin.
         */
        const ImageRect &getBorder() const { return mBorder; }

        /**
         * Returns the image used by a close button for this skin.
         */
        Image *getCloseImage() const { return mCloseImage; }

        /**
         * Returns the image used by a sticky button for this skin.
         */
        Image *getStickyImage(bool state) const
        { return state ? mStickyImageDown.get() : mStickyImageUp.get(); }

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

        int instances = 0;

    private:
        ImageRect mBorder;              /**< The window border and background */
        ResourceRef<Image> mCloseImage; /**< Close Button Image */
        std::unique_ptr<Image> mStickyImageUp;          /**< Sticky Button Image */
        std::unique_ptr<Image> mStickyImageDown;        /**< Sticky Button Image */
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

        enum ArrowButtonDirection {
            ARROW_UP,
            ARROW_DOWN,
            ARROW_LEFT,
            ARROW_RIGHT
        };

        enum ThemePalette {
            TEXT,
            SHADOW,
            OUTLINE,
            PROGRESS_BAR,
            BUTTON,
            BUTTON_DISABLED,
            TAB,
            PARTY_CHAT_TAB,
            PARTY_SOCIAL_TAB,
            BACKGROUND,
            HIGHLIGHT,
            TAB_FLASH,
            SHOP_WARNING,
            ITEM_EQUIPPED,
            CHAT,
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

        /**
         * Loads a skin.
         */
        Skin *load(const std::string &filename);

        struct WidgetState
        {
            int width = 0;
            int height = 0;
            bool enabled = true;
            bool hovered = false;
            bool selected = false;
            bool focused = false;
        };

        void drawButton(Graphics *graphics, const WidgetState &state) const;
        void drawTextFieldFrame(Graphics *graphics, const WidgetState &state) const;
        void drawTab(Graphics *graphics, const WidgetState &state) const;
        void drawCheckBox(gcn::Graphics *graphics, const WidgetState &state) const;
        void drawRadioButton(gcn::Graphics *graphics, const WidgetState &state) const;
        void drawSlider(Graphics *graphics, const WidgetState &state, int markerPosition) const;
        void drawDropDownFrame(Graphics *graphics, const WidgetState &state) const;
        void drawDropDownButton(Graphics *graphics, const WidgetState &state) const;
        void drawProgressBar(Graphics *graphics,
                             const gcn::Rectangle &area,
                             const gcn::Color &color,
                             float progress,
                             const std::string &text = std::string()) const;
        void drawScrollAreaFrame(Graphics *graphics, const WidgetState &state) const;
        void drawScrollAreaButton(Graphics *graphics,
                                  ArrowButtonDirection dir,
                                  bool pressed,
                                  const gcn::Rectangle &dim) const;
        void drawScrollAreaMarker(Graphics *graphics, bool hovered, const gcn::Rectangle &dim) const;

        int getSliderMarkerLength() const;
        const Image *getResizeGripImage() const { return mResizeGripImage; }

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

        std::unique_ptr<Skin> readSkin(const std::string &filename) const;

        ResourceRef<Image> getImage(const std::string &path) const;

        // Map containing all window skins
        std::map<std::string, std::unique_ptr<Skin>> mSkins;

        std::string mThemePath;

        void loadColors();

        /**
         * Tells if the current skins opacity
         * should not get less than the given value
         */
        float mMinimumOpacity = 0.0f;
        float mAlpha = 1.0;

        std::vector<std::unique_ptr<DyePalette>> mProgressColors;

        ImageRect *mButton;     /**< Button state graphics */
        ImageRect mTabImg[4];   /**< Tab state graphics */
        ImageRect mDeepBoxImageRect;

        std::unique_ptr<Image> mCheckBoxNormal;
        std::unique_ptr<Image> mCheckBoxChecked;
        std::unique_ptr<Image> mCheckBoxDisabled;
        std::unique_ptr<Image> mCheckBoxDisabledChecked;
        std::unique_ptr<Image> mCheckBoxNormalHi;
        std::unique_ptr<Image> mCheckBoxCheckedHi;

        ResourceRef<Image> mRadioNormal;
        ResourceRef<Image> mRadioChecked;
        ResourceRef<Image> mRadioDisabled;
        ResourceRef<Image> mRadioDisabledChecked;
        ResourceRef<Image> mRadioNormalHi;
        ResourceRef<Image> mRadioCheckedHi;

        std::unique_ptr<Image> hStart, hMid, hEnd, hGrip;
        std::unique_ptr<Image> vStart, vMid, vEnd, vGrip;
        std::unique_ptr<Image> hStartHi, hMidHi, hEndHi, hGripHi;
        std::unique_ptr<Image> vStartHi, vMidHi, vEndHi, vGripHi;

        ImageRect mScrollBarMarker;
        ImageRect mScrollBarMarkerHi;
        ResourceRef<Image> mArrowButtons[4][2];

        ResourceRef<Image> mResizeGripImage;
};
