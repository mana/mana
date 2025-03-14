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
#include "resources/resource.h"

#include <map>
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
        { return state ? mStickyImageDown : mStickyImageUp; }

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
        void updateAlpha(float minimumOpacityAllowed = 0.0f);

        int instances = 0;

    private:
        ImageRect mBorder;              /**< The window border and background */
        ResourceRef<Image> mCloseImage; /**< Close Button Image */
        Image *mStickyImageUp;          /**< Sticky Button Image */
        Image *mStickyImageDown;        /**< Sticky Button Image */
};

class Theme : public Palette, public EventListener
{
    public:
        static Theme *instance();
        static void deleteInstance();

        static void prepareThemePath();
        static const std::string &getThemePath() { return mThemePath; }

        /**
         * Returns the patch to the given gui resource relative to the theme
         * or, if it isn't in the theme, relative to 'graphics/gui'.
         */
        static std::string resolveThemePath(const std::string &path);

        static ResourceRef<Image> getImageFromTheme(const std::string &path);

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
        static const gcn::Color &getThemeColor(int type, int alpha = 255)
        {
            return mInstance->getColor(type, alpha);
        }

        static const gcn::Color &getThemeColor(char c, bool &valid)
        {
            return mInstance->getColor(c, valid);
        }

        static gcn::Color getProgressColor(int type, float progress);

        /**
         * Loads a skin.
         */
        Skin *load(const std::string &filename,
                   const std::string &defaultPath = getThemePath());

        /**
         * Updates the alpha values of all of the skins.
         */
        void updateAlpha();

        /**
         * Get the minimum opacity allowed to skins.
         */
        float getMinimumOpacity() const
        { return mMinimumOpacity; }

        /**
         * Set the minimum opacity allowed to skins.
         * Set a negative value to free the minimum allowed.
         */
        void setMinimumOpacity(float minimumOpacity);

        void event(Event::Channel channel, const Event &event) override;

    private:
        Theme();
        ~Theme() override;

        Skin *readSkin(const std::string &filename);

        // Map containing all window skins
        std::map<std::string, Skin *> mSkins;

        static std::string mThemePath;
        static Theme *mInstance;

        static bool tryThemePath(std::string themePath);

        void loadColors(std::string file = std::string());

        /**
         * Tells if the current skins opacity
         * should not get less than the given value
         */
        float mMinimumOpacity;

        std::vector<DyePalette *> mProgressColors;
};
