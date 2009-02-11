/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#ifndef MAPREADER_H
#define MAPREADER_H

#include <libxml/tree.h>

class Map;
class Properties;
class Tileset;

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
        static Map *readMap(xmlNodePtr node, const std::string &path);

    private:
        /**
         * Reads the properties element.
         *
         * @param node  The <code>properties</code> element.
         * @param props The Properties instance to which the properties will
         *              be assigned.
         */
        static void readProperties(xmlNodePtr node, Properties* props);

        /**
         * Reads a map layer and adds it to the given map.
         */
        static void readLayer(xmlNodePtr node, Map *map);

        /**
         * Reads a tile set.
         */
        static Tileset *readTileset(xmlNodePtr node, const std::string &path,
                                    Map *map);

        /**
         * Gets an integer property from an xmlNodePtr.
         */
        static int getProperty(xmlNodePtr node, const char* name, int def);
};

#endif
