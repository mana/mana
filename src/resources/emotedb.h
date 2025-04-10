/*
 *  Emote database
 *  Copyright (C) 2009  Aethyra Development Team
 *  Copyright (C) 2009-2013  The Mana Developers
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

#include <string>

#include "resources/image.h"
#include "resources/imageset.h"

#include "utils/xml.h"

struct Emote
{
    int id;
    int effectId;
    std::string name;
    ResourceRef<ImageSet> is;
    ResourceRef<Image> image;
};

/**
 * Emote information database.
 */
namespace EmoteDB
{
    void init();

    void readEmoteNode(XML::Node node, const std::string &filename);

    void checkStatus();

    void unload();

    const Emote &get(int id);
    const Emote &getByIndex(int index);

    int getEmoteCount();
}
