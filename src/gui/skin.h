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
        Skin();
        ~Skin();

        std::string mName;         /**< Name of the skin to use */
        ImageRect border;          /**< The window border and background */
        Image *closeImage;         /**< Close Button Image */
        int instances;
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
