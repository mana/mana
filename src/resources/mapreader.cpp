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

#include <cassert>
#include <iostream>
#include <zlib.h>

#include "resourcemanager.h"
#include "image.h"

#include "../base64.h"
#include "../log.h"
#include "../map.h"
#include "../tileset.h"

#include "../utils/tostring.h"

const unsigned int DEFAULT_TILE_WIDTH = 32;
const unsigned int DEFAULT_TILE_HEIGHT = 32;

/**
 * Inflates either zlib or gzip deflated memory. The inflated memory is
 * expected to be freed by the caller.
 */
int
inflateMemory(unsigned char *in, unsigned int inLength,
              unsigned char *&out, unsigned int &outLength)
{
    int bufferSize = 256 * 1024;
    int ret;
    z_stream strm;

    out = (unsigned char*)malloc(bufferSize);

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = in;
    strm.avail_in = inLength;
    strm.next_out = out;
    strm.avail_out = bufferSize;

    ret = inflateInit2(&strm, 15 + 32);

    if (ret != Z_OK)
        return ret;

    do
    {
        if (strm.next_out == NULL)
        {
            inflateEnd(&strm);
            return Z_MEM_ERROR;
        }

        ret = inflate(&strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);

        switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
        }

        if (ret != Z_STREAM_END)
        {
            out = (unsigned char*)realloc(out, bufferSize * 2);

            if (out == NULL)
            {
                inflateEnd(&strm);
                return Z_MEM_ERROR;
            }

            strm.next_out = out + bufferSize;
            strm.avail_out = bufferSize;
            bufferSize *= 2;
        }
    }
    while (ret != Z_STREAM_END);
    assert(strm.avail_in == 0);

    outLength = bufferSize - strm.avail_out;
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

Map*
MapReader::readMap(const std::string &filename)
{
    // Load the file through resource manager
    ResourceManager *resman = ResourceManager::getInstance();
    int fileSize;
    void *buffer = resman->loadFile(filename, fileSize);
    Map *map = NULL;

    if (buffer == NULL)
    {
        logger->log("Map file not found (%s)\n", filename.c_str());
        return NULL;
    }

    // Inflate the gzipped map data
    unsigned char *inflated;
    unsigned int inflatedSize = 0;
    int ret = inflateMemory((unsigned char*)buffer,
                            fileSize, inflated, inflatedSize);
    free(buffer);

    if (ret == Z_MEM_ERROR)
    {
        logger->log("Error: Out of memory while decompressing map data!");
        return NULL;
    }
    else if (ret == Z_VERSION_ERROR)
    {
        logger->log("Error: Incompatible zlib version!");
        return NULL;
    }
    else if (ret == Z_DATA_ERROR)
    {
        logger->log("Error: Incorrect zlib compressed data!");
        return NULL;
    }
    else if (ret != Z_OK || inflated == NULL)
    {
        logger->log("Error: Unknown error while decompressing map data!");
        return NULL;
    }

    xmlDocPtr doc = xmlParseMemory((char*)inflated, inflatedSize);
    free(inflated);

    // Parse the inflated map data
    if (doc) {
        xmlNodePtr node = xmlDocGetRootElement(doc);

        if (!node || !xmlStrEqual(node->name, BAD_CAST "map")) {
            logger->log("Error: Not a map file (%s)!", filename.c_str());
            return NULL;
        }
        else
        {
            map = readMap(node, filename);
        }
        xmlFreeDoc(doc);
    } else {
        logger->log("Error while parsing map file (%s)!", filename.c_str());
    }

    return map;
}

Map*
MapReader::readMap(xmlNodePtr node, const std::string &path)
{
    xmlChar *prop;

    // Take the filename off the path
    std::string pathDir = path.substr(0, path.rfind("/") + 1);

    prop = xmlGetProp(node, BAD_CAST "version");
    xmlFree(prop);

    int w = getProperty(node, "width", 0);
    int h = getProperty(node, "height", 0);
    int tilew = getProperty(node, "tilewidth", DEFAULT_TILE_WIDTH);
    int tileh = getProperty(node, "tileheight", DEFAULT_TILE_HEIGHT);
    int layerNr = 0;
    Map *map = new Map(w, h, tilew, tileh);

    for (node = node->xmlChildrenNode; node != NULL; node = node->next)
    {
        if (xmlStrEqual(node->name, BAD_CAST "tileset"))
        {
            Tileset *tileset = readTileset(node, pathDir, map);
            if (tileset) {
                map->addTileset(tileset);
            }
        }
        else if (xmlStrEqual(node->name, BAD_CAST "layer"))
        {
            logger->log("- Loading layer %d", layerNr);
            readLayer(node, map, layerNr);
            layerNr++;
        }
        else if (xmlStrEqual(node->name, BAD_CAST "properties"))
        {
            readProperties(node, map);
        }
    }

    //set Overlays
    ResourceManager *resman = ResourceManager::getInstance();
    for (int i = 0; ; i++)
    {
        const std::string name = "overlay" + toString(i);

        if (!map->hasProperty(name + "image"))
            break; // Finished

        Image *img = resman->getImage(map->getProperty(name + "image"));
        float scrollX = 0.0f;
        float scrollY = 0.0f;
        float parallax = 0.0f;
        std::stringstream ss;

        if (map->hasProperty(name + "scrollX"))
        {
            ss.str(map->getProperty(name + "scrollX"));
            ss >> scrollX;
        }
        if (map->hasProperty(name + "scrollY"))
        {
            ss.str(map->getProperty(name + "scrollY"));
            ss >> scrollY;
        }
        if (map->hasProperty(name + "parallax"))
        {
            ss.str(map->getProperty(name + "parallax"));
            ss >> parallax;
        }
        map->setOverlay(img, scrollX, scrollY, parallax);
        img->decRef();
    }

    return map;
}

void
MapReader::readProperties(xmlNodePtr node, Properties* props)
{
    for (node = node->xmlChildrenNode; node; node = node->next) {
        if (!xmlStrEqual(node->name, BAD_CAST "property"))
            continue;

        // Example: <property name="name" value="value"/>
        xmlChar *name = xmlGetProp(node, BAD_CAST "name");
        xmlChar *value = xmlGetProp(node, BAD_CAST "value");

        if (name && value) {
            props->setProperty((const char*)name, (const char*)value);
        }

        if (name) xmlFree(name);
        if (value) xmlFree(value);
    }
}

void
MapReader::readLayer(xmlNodePtr node, Map *map, int layer)
{
    int h = map->getHeight();
    int w = map->getWidth();
    int x = 0;
    int y = 0;

    // Load the tile data. Layers are assumed to be map size, with (0,0) as
    // origin.
    for (node = node->xmlChildrenNode; node; node = node->next) {
        if (!xmlStrEqual(node->name, BAD_CAST "data"))
            continue;

        xmlChar *encoding = xmlGetProp(node, BAD_CAST "encoding");
        xmlChar *compression = xmlGetProp(node, BAD_CAST "compression");

        if (encoding && xmlStrEqual(encoding, BAD_CAST "base64"))
        {
            xmlFree(encoding);

            if (compression) {
                logger->log("Warning: no layer compression supported!");
                xmlFree(compression);
                return;
            }

            // Read base64 encoded map file
            xmlNodePtr dataChild = node->xmlChildrenNode;
            if (!dataChild)
                continue;

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

                    map->setTileWithGid(x, y, layer, gid);

                    x++;
                    if (x == w) {x = 0; y++;}
                }
                free(binData);
            }
        }
        else {
            // Read plain XML map file
            for (xmlNodePtr n2 = node->xmlChildrenNode; n2; n2 = n2->next) {
                if (!xmlStrEqual(n2->name, BAD_CAST "tile"))
                    continue;

                int gid = getProperty(n2, "gid", -1);
                map->setTileWithGid(x, y, layer, gid);

                x++;
                if (x == w) {
                    x = 0; y++;
                    if (y >= h)
                        break;
                }
            }
        }

        if (y < h)
            std::cerr << "TOO SMALL!\n";
        if (x)
            std::cerr << "TOO SMALL!\n";

        // There can be only one data element
        break;
    }
}

Tileset*
MapReader::readTileset(xmlNodePtr node,
                       const std::string &path,
                       Map *map)
{
    if (xmlHasProp(node, BAD_CAST "source"))
    {
        logger->log("Warning: External tilesets not supported yet.");
        return NULL;
    }

    int firstGid = getProperty(node, "firstgid", 0);
    int tw = getProperty(node, "tilewidth", map->getTileWidth());
    int th = getProperty(node, "tileheight", map->getTileHeight());

    for (node = node->xmlChildrenNode; node; node = node->next) {
        if (!xmlStrEqual(node->name, BAD_CAST "image"))
            continue;

        xmlChar* source = xmlGetProp(node, BAD_CAST "source");

        if (source)
        {
            std::string sourceStr = std::string((const char*)source);
            sourceStr.erase(0, 3);  // Remove "../"

            ResourceManager *resman = ResourceManager::getInstance();
            Image* tilebmp = resman->getImage(sourceStr);

            if (tilebmp)
            {
                Tileset *set = new Tileset(tilebmp, tw, th, firstGid);
                tilebmp->decRef();
                xmlFree(source);
                return set;
            }
            else {
                logger->log("Warning: Failed to load tileset (%s)", source);
            }
        }

        break;
    }

    return NULL;
}

int
MapReader::getProperty(xmlNodePtr node, const char* name, int def)
{
    int &ret = def;

    xmlChar *prop = xmlGetProp(node, BAD_CAST name);
    if (prop) {
        ret = atoi((char*)prop);
        xmlFree(prop);
    }
    return ret;
}
