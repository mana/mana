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
    xmlFree(prop);

    int w = getProperty(node, "width", 0);
    int h = getProperty(node, "height", 0);
    int tilew = getProperty(node, "tilewidth", DEFAULT_TILE_WIDTH);
    int tileh = getProperty(node, "tileheight", DEFAULT_TILE_HEIGHT);
    int layerNr = 0;
    Map* map = new Map(w, h);

    for (node = node->xmlChildrenNode; node != NULL; node = node->next)
    {
        if (xmlStrEqual(node->name, BAD_CAST "tileset")) {
            /*
            Tileset* set = readTileset(node, path);
            if (set) {
                map->tilesets.push_back(set);
            }
            */
        }
        else if (xmlStrEqual(node->name, BAD_CAST "layer")) {
            log("- Loading layer %d", layerNr);
            readLayer(node, map, layerNr);
            layerNr++;
        }
        /*
        else if (xmlStrEqual(node->name, BAD_CAST "object")) {
            int x = getProperty(node, "x", 0);
            int y = getProperty(node, "y", 0);

            // Spawn the object
            prop = xmlGetProp(node, BAD_CAST "type");
            
            log("- Adding %s at (%d, %d)", (char*)prop, x, y);
            map->addObject(double(x) / tilew, double(y) / tileh, (char*)prop);
            xmlFree(prop);
        }
        */
    }

    return map;
}

void MapReader::readLayer(xmlNodePtr node, Map *map, int layer)
{
    // Ignored layer attributes
    //int w = getProperty(node, "width", 0);   // Map width is used
    //int h = getProperty(node, "height", 0);  // Map height is used
    //xmlChar *name = xmlGetProp(node, BAD_CAST "name"); 

    node = node->xmlChildrenNode;
    int x = 0;
    int y = 0;
    
    // Load the tile data
    /*
    while (node != NULL)
    {
        if (xmlStrEqual(node->name, BAD_CAST "tile") && y < h)
        {
            xmlChar *name = xmlGetProp(node, BAD_CAST "name");
            int gid = getProperty(node, "gid", -1);

            if (gid > -1) {
                layer->setTile(Point(x, y), map->getTile(gid));
            }
            else {
                layer->setTile(Point(x, y), NULL);
            }

            x++;
            if (x == w) {x = 0; y++;}
        } 
    
        if (xmlStrEqual(node->name, BAD_CAST "data")) {
            node = node->xmlChildrenNode;
        } else {
            node = node->next;
        }
    }
    */
}

/*
Tileset* MapReader::readTileset(xmlNodePtr node, const std::string &path)
{
    Tileset* set = NULL;

    xmlChar* prop = xmlGetProp(node, BAD_CAST "source");
    if (prop) {
        console.log(CON_LOG, CON_ALWAYS,
                "Warning: External tilesets not supported yet.");
        xmlFree(prop);
        return NULL;
    }

    int firstgid = getProperty(node, "firstgid", 0);
    int tw = getProperty(node, "tilewidth", DEFAULT_TILE_WIDTH);
    int th = getProperty(node, "tileheight", DEFAULT_TILE_HEIGHT);
    int ts = getProperty(node, "spacing", 0);
    xmlChar* name = xmlGetProp(node, BAD_CAST "name");

    node = node->xmlChildrenNode;

    while (node != NULL) {
        if (xmlStrEqual(node->name, BAD_CAST "image")) {
            xmlChar* source = xmlGetProp(node, BAD_CAST "source");

            if (source) {
                char* srcname = get_filename((char*)source);
                BITMAP* tilebmp = module->findBitmap(srcname);

                if (tilebmp) {
                    set = new Tileset();
                    set->importTileBitmap(tilebmp, (char*)source, tw, th, ts);
                    set->setFirstGid(firstgid);
                    if (name) { set->setName((char*)name); }
                    xmlFree(source);
                } else {
                    printf("Warning: Failed to load tileset %s (%s)\n",
                            srcname, (char*)name);
                }
            }

            break;
        }

        node = node->next;
    }

    xmlFree(name);

    return set;
}
*/

int MapReader::getProperty(xmlNodePtr node, const char* name, int def)
{
    xmlChar *prop = xmlGetProp(node, BAD_CAST name);
    if (prop) {
        int val = atoi((char*)prop);
        xmlFree(prop);
        return val;
    }
    else {
        return def;
    }
}
