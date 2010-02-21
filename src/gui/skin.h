/*
 *  Gui Skinning
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2009  Aethyra Development Team
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef SKIN_H
#define SKIN_H

#include "graphics.h"

#include <map>
#include <string>

class ConfigListener;
class Image;

class Skin
{
    public:
        Skin(ImageRect skin, Image *close, Image *stickyUp, Image *stickyDown,
             const std::string &filePath,
             const std::string &name = "");

        ~Skin();

        /**
         * Returns the skin's name. Useful for giving a human friendly skin
         * name if a dialog for skin selection for a specific window type is
         * done.
         */
        const std::string &getName() const { return mName; }

        /**
         * Returns the skin's xml file path.
         */
        const std::string &getFilePath() const { return mFilePath; }

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

        int instances;

    private:
        std::string mFilePath;     /**< File name path for the skin */
        std::string mName;         /**< Name of the skin to use */
        ImageRect mBorder;         /**< The window border and background */
        Image *mCloseImage;        /**< Close Button Image */
        Image *mStickyImageUp;     /**< Sticky Button Image */
        Image *mStickyImageDown;   /**< Sticky Button Image */
};

class SkinLoader
{
    public:
        static SkinLoader *instance();
        static void deleteInstance();

        /**
         * Loads a skin.
         */
        Skin *load(const std::string &filename,
                   const std::string &defaultPath);

        /**
         * Updates the alpha values of all of the skins.
         */
        void updateAlpha();

        /**
         * Get the minimum opacity allowed to skins.
         */
        float getMinimumOpacity()
        { return mMinimumOpacity; }

        /**
         * Set the minimum opacity allowed to skins.
         * Set a negative value to free the minimum allowed.
         */
        void setMinimumOpacity(float minimumOpacity);

    private:
        SkinLoader();
        ~SkinLoader();

        Skin *readSkin(const std::string &filename);

        // Map containing all window skins
        typedef std::map<std::string, Skin*> Skins;
        typedef Skins::iterator SkinIterator;

        Skins mSkins;

        /**
         * The config listener that listens to changes relevant to all skins.
         */
        ConfigListener *mSkinConfigListener;

        static SkinLoader *mInstance;

        /**
         * Tells if the current skins opacity
         * should not get less than the given value
         */
        float mMinimumOpacity;
};

#endif
