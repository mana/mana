/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include "mapreader.h"
#include "../log.h"
#include "resourcemanager.h"
#include "../graphic/spriteset.h"

#include <iostream>

#define DEFAULT_TILE_WIDTH  32
#define DEFAULT_TILE_HEIGHT 32


Map *MapReader::readMap(const std::string &filename)
{
    std::cout << "Attempting to parse XML map data";

    FILE* f = fopen(filename.c_str(), "rb");
    char *map_string;

    if (!f) {
        error(std::string("Error: failed to open ") + filename);
    }

    // Get size of file
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    // Read file into character array
    map_string = new char[size + 1];
    fread(map_string, 1, size, f);
    map_string[size] = '\0';

    fclose(f);

    xmlDocPtr doc = xmlReadMemory(map_string, size, NULL, NULL, 0);
    delete[] map_string;

    if (doc) {
        std::cout << "Looking for root node";
        xmlNodePtr node = xmlDocGetRootElement(doc);

        if (!node || !xmlStrEqual(node->name, BAD_CAST "map")) {
            log("Warning: No map file (%s)!", filename.c_str());
            return NULL;
        }
        
        std::cout << "Loading map from XML tree";
        return readMap(node, filename);
        xmlFreeDoc(doc);
    } else {
        log("Error while parsing map file (%s)!", filename.c_str());
    }

    return NULL;
}

Map* MapReader::readMap(xmlNodePtr node, const std::string &path)
{
    xmlChar *prop;

    prop = xmlGetProp(node, BAD_CAST "version");
#ifndef WIN32
    xmlFree(prop);
#endif

    int w = getProperty(node, "width", 0);
    int h = getProperty(node, "height", 0);
    int tilew = getProperty(node, "tilewidth", DEFAULT_TILE_WIDTH);
    int tileh = getProperty(node, "tileheight", DEFAULT_TILE_HEIGHT);
    int layerNr = 0;
    Map* map = new Map(w, h);

    for (node = node->xmlChildrenNode; node != NULL; node = node->next)
    {
        if (xmlStrEqual(node->name, BAD_CAST "tileset"))
        {
            readTileset(node, path, map);
        }
        else if (xmlStrEqual(node->name, BAD_CAST "layer"))
        {
            log("- Loading layer %d", layerNr);
            readLayer(node, map, layerNr);
            layerNr++;
        }
    }

    return map;
}

void MapReader::readLayer(xmlNodePtr node, Map *map, int layer)
{
    node = node->xmlChildrenNode;
    int h = map->getHeight();
    int w = map->getWidth();
    int x = 0;
    int y = 0;
    
    // Load the tile data. Layers are assumed to be map size, with (0,0) as
    // origin.
    while (node != NULL)
    {
        if (xmlStrEqual(node->name, BAD_CAST "tile") && y < h)
        {
            int gid = getProperty(node, "gid", -1);
            map->setTile(x, y, layer, (gid > -1) ? gid : 0);

            x++;
            if (x == w) {x = 0; y++;}
        } 
    
        if (xmlStrEqual(node->name, BAD_CAST "data")) {
            node = node->xmlChildrenNode;
        } else {
            node = node->next;
        }
    }
}

void MapReader::readTileset(xmlNodePtr node, const std::string &path, Map *map)
{
    xmlChar* prop = xmlGetProp(node, BAD_CAST "source");
    if (prop) {
        log("Warning: External tilesets not supported yet.");
#ifndef WIN32
        xmlFree(prop);
#endif
        return;
    }

    int firstgid = getProperty(node, "firstgid", 0);
    int tw = getProperty(node, "tilewidth", map->getTileWidth());
    int th = getProperty(node, "tileheight", map->getTileHeight());

    node = node->xmlChildrenNode;

    while (node != NULL)
    {
        if (xmlStrEqual(node->name, BAD_CAST "image"))
        {
            xmlChar* source = xmlGetProp(node, BAD_CAST "source");

            if (source)
            {
                ResourceManager *resman = ResourceManager::getInstance();
                Image* tilebmp = resman->getImage(path +
                        std::string((const char*)source));

                if (tilebmp)
                {
                    Spriteset *set = new Spriteset(tilebmp, tw, th);
                    //set->setFirstGid(firstgid);
                    // TODO: Like uhm, do something with this set!
#ifndef WIN32
                    xmlFree(source);
#endif
                }
                else {
                    log("Warning: Failed to load tileset (%s)\n", source);
                }
            }

            break;
        }

        node = node->next;
    }
}

int MapReader::getProperty(xmlNodePtr node, const char* name, int def)
{
    xmlChar *prop = xmlGetProp(node, BAD_CAST name);
    if (prop) {
        int val = atoi((char*)prop);
#ifndef WIN32
        xmlFree(prop);
#endif
        return val;
    }
    else {
        return def;
    }
}
