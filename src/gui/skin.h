/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra.
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

#include <map>
#include <string>

#include "../graphics.h"

class Image;

class Skin
{
    public:
        Skin(ImageRect skin, Image* close, std::string name = "");
        ~Skin();

        /**
         * Returns the skin's name. Useful for giving a human friendly skin
         * name if a dialog for skin selection for a specific window type is
         * done.
         */
        std::string getName() { return mName; }

        /**
         * Returns the background skin.
         */
        ImageRect getBorder() { return border; }

        /**
         * Returns the image used by a close button for this skin.
         */
        Image* getCloseImage() { return closeImage; }

        /**
         * Returns the number of instances which use this skin.
         */
        int getNumberOfInstances() { return instances; }

        /**
         * Returns the minimum width which can be used with this skin.
         */
        unsigned int getMinWidth();

        /**
         * Returns the minimum height which can be used with this skin.
         */
        unsigned int getMinHeight();

        int instances;

    private:
        std::string mName;         /**< Name of the skin to use */
        ImageRect border;          /**< The window border and background */
        Image *closeImage;         /**< Close Button Image */
};

// Map containing all window skins
typedef std::map<std::string, Skin*> Skins;

// Iterator for window skins
typedef Skins::iterator SkinIterator;

class SkinLoader 
{
    public:
        SkinLoader();
        ~SkinLoader();

        /**
         * Loads a skin
         */
        Skin* load(const std::string &filename);

    private:
        Skins mSkins;
};

extern SkinLoader* skinLoader;

#endif
