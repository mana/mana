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
#include "../base64.h"

#include <iostream>

#define DEFAULT_TILE_WIDTH  32
#define DEFAULT_TILE_HEIGHT 32

// MSVC libxml2 at the moment doesn't work right when using MinGW, missing this
// function at link time.
#ifdef WIN32
#undef xmlFree
#define xmlFree(x) ;
#endif

std::vector<Tileset*> MapReader::tilesets;

Tileset::Tileset(Image *img, int w, int h, int firstGid):
    Spriteset(img, w, h),
    firstGid(firstGid)
{
}

int Tileset::getFirstGid()
{
    return firstGid;
}


Map *MapReader::readMap(const std::string &filename)
{
    log("Attempting to parse XML map data");

    std::string name = std::string("data/") + filename;

    FILE* f = fopen(name.c_str(), "rb");
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
        log("Looking for root node");
        xmlNodePtr node = xmlDocGetRootElement(doc);

        if (!node || !xmlStrEqual(node->name, BAD_CAST "map")) {
            log("Warning: No map file (%s)!", filename.c_str());
            return NULL;
        }
        
        log("Loading map from XML tree");
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
    
    // Take the filename off the path
    std::string pathDir = path.substr(0, path.rfind("/") + 1);

    prop = xmlGetProp(node, BAD_CAST "version");
    xmlFree(prop);

    int w = getProperty(node, "width", 0);
    int h = getProperty(node, "height", 0);
    // We only support tile width of 32 at the moment
    //int tilew = getProperty(node, "tilewidth", DEFAULT_TILE_WIDTH);
    //int tileh = getProperty(node, "tileheight", DEFAULT_TILE_HEIGHT);
    int layerNr = 0;
    Map* map = new Map(w, h);

    for (node = node->xmlChildrenNode; node != NULL; node = node->next)
    {
        if (xmlStrEqual(node->name, BAD_CAST "tileset"))
        {
            Tileset *tileset = readTileset(node, pathDir, map);
            if (tileset) {
                tilesets.push_back(tileset);
            }
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
        if (xmlStrEqual(node->name, BAD_CAST "data"))
        {
            xmlChar *encoding = xmlGetProp(node, BAD_CAST "encoding");
            xmlChar *compression = xmlGetProp(node, BAD_CAST "compression");

            if (encoding && xmlStrEqual(encoding, BAD_CAST "base64"))
            {
                xmlFree(encoding);

                if (compression) {
                    log("Warning: no layer compression supported!");
                    xmlFree(compression);
                    return;
                }

                // Read base64 encoded map file
                xmlNodePtr dataChild = node->xmlChildrenNode;
                if (!dataChild) continue;

                int len = strlen((const char*)dataChild->content) + 1;
                unsigned char *charData = new unsigned char[len + 1];
                const char *charStart = (const char*)dataChild->content;
                unsigned char *charIndex = charData;

                while (*charStart) {
                    if (*charStart != ' ' && *charStart != '\t' &&
                            *charStart != '\n')
                    {
                        *charIndex = *charStart;
                        charIndex++;
                    }
                    charStart++;
                }
                *charIndex = '\0';

                int binLen;
                unsigned char *binData =
                    php_base64_decode(charData, strlen((char*)charData),
                            &binLen);

                delete[] charData;

                if (binData) {
                    for (int i = 0; i < binLen - 3; i += 4) {
                        int gid = binData[i] |
                            binData[i + 1] << 8 |
                            binData[i + 2] << 16 |
                            binData[i + 3] << 24;

                        setTileWithGid(map, x, y, layer, gid);

                        x++;
                        if (x == w) {x = 0; y++;}
                    }
                    free(binData);
                }
            }
            else {
                // Read plain XML map file
                xmlNodePtr n2 = node->xmlChildrenNode;

                while (n2 != NULL)
                {
                    if (xmlStrEqual(n2->name, BAD_CAST "tile") && y < h)
                    {
                        int gid = getProperty(n2, "gid", -1);
                        setTileWithGid(map, x, y, layer, gid);

                        x++;
                        if (x == w) {x = 0; y++;}
                    }

                    n2 = n2->next;
                }
            }

            // There can be only one data element
            break;
        }

        node = node->next;
    }
}

Tileset* MapReader::readTileset(
        xmlNodePtr node, const std::string &path, Map *map)
{
    if (xmlHasProp(node, BAD_CAST "source")) {
        log("Warning: External tilesets not supported yet.");
        return NULL;
    }

    int firstGid = getProperty(node, "firstgid", 0);
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
                    Tileset *set = new Tileset(tilebmp, tw, th, firstGid);
                    xmlFree(source);
                    return set;
                }
                else {
                    log("Warning: Failed to load tileset (%s)\n", source);
                }
            }

            break;
        }

        node = node->next;
    }

    return NULL;
}

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

Image *MapReader::getTileWithGid(int gid)
{
    Tileset *set = getTilesetWithGid(gid);

    if (set) {
        return set->spriteset[gid - set->getFirstGid()];
    }

    return NULL;
}

Tileset *MapReader::getTilesetWithGid(int gid)
{
    std::vector<Tileset*>::iterator i;
    Tileset *set = NULL;

    // Find the tileset with the highest firstGid below/eq to gid
    for (i = tilesets.begin(); i != tilesets.end(); ++i)
    {
        if ((*i)->getFirstGid() <= gid) {
            set = (*i);
        }
        else {
            break;
        }
    }

    if (set && (gid - set->getFirstGid()) < (int)set->spriteset.size())
    {
        return set;
    }

    return NULL;
}

void MapReader::setTileWithGid(Map *map, int x, int y, int layer, int gid)
{
    if (layer == 3)
    {
        Tileset *set = getTilesetWithGid(gid);
        map->setWalk(x, y,
                !set || (gid - set->getFirstGid() == 0));
    }
    else if (layer < 3) {
        map->setTile(x, y, layer, getTileWithGid(gid));
    }
}
