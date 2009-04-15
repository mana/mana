/*
 *  Gui Skinning
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2009  Aethyra Development Team
 *  Copyright (C) 2009  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
        void updateAlpha();

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
};

#endif
