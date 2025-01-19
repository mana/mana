/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#ifndef MAPREADER_H
#define MAPREADER_H

#include "utils/xml.h"

#include <string>

class Map;

/**
 * Reader for XML map files (*.tmx)
 */
class MapReader
{
public:
    /**
     * Read an XML map from a file.
     */
    static Map *readMap(const std::string &filename);

    /**
     * Read an XML map from a parsed XML tree. The path is used to find the
     * location of referenced tileset images.
     */
    static Map *readMap(XML::Node node, const std::string &path);
};

#endif // MAPREADER_H
