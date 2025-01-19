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

#include "resources/mapreader.h"

#include "configuration.h"
#include "log.h"
#include "map.h"
#include "tileset.h"

#include "resources/animation.h"
#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "utils/base64.h"
#include "utils/stringutils.h"
#include "utils/zlib.h"

#include <iostream>

static void readProperties(XML::Node node, Properties* props);

static void readLayer(XML::Node node, Map *map);

static Tileset *readTileset(XML::Node node,
                            const std::string &path,
                            Map *map);

static void readTileAnimation(XML::Node tileNode,
                              Tileset *set,
                              unsigned tileGID,
                              Map *map);

static std::string resolveRelativePath(std::string base, std::string relative)
{
    // Remove trailing "/", if present
    size_t i = base.length();
    if (base.at(i - 1) == '/')
        base.erase(i - 1, i);

    while (relative.substr(0, 3) == "../")
    {
        relative.erase(0, 3);  // Remove "../"
        if (!base.empty()) // If base is already empty, we can't trim anymore
        {
            i = base.find_last_of('/');
            if (i == std::string::npos)
                i = 0;
            base.erase(i, base.length()); // Remove deepest folder in base
        }
    }

    // Re-add trailing slash, if needed
    if (!base.empty() && base[base.length() - 1] != '/')
        base += '/';

    return base + relative;
}

Map *MapReader::readMap(const std::string &filename)
{
    logger->log("Attempting to read map %s", filename.c_str());
    Map *map = nullptr;

    XML::Document doc(filename);

    XML::Node node = doc.rootNode();

    // Parse the inflated map data
    if (node)
    {
        if (node.name() != "map")
        {
            logger->log("Error: Not a map file (%s)!", filename.c_str());
        }
        else
        {
            map = readMap(node, filename);
        }
    }
    else
    {
        logger->log("Error while parsing map file (%s)!", filename.c_str());
    }

    if (map)
        map->setProperty("_filename", filename);

    return map;
}

Map *MapReader::readMap(XML::Node node, const std::string &path)
{
    // Take the filename off the path
    const std::string pathDir = path.substr(0, path.rfind("/") + 1);

    const int w = node.getProperty("width", 0);
    const int h = node.getProperty("height", 0);
    const int tilew = node.getProperty("tilewidth", -1);
    const int tileh = node.getProperty("tileheight", -1);

    if (tilew < 0 || tileh < 0)
    {
        logger->log("MapReader: Warning: "
                    "Unitialized tile width or height value for map: %s",
                    path.c_str());
        return nullptr;
    }

    Map *map = new Map(w, h, tilew, tileh);

    for (auto childNode : node.children())
    {
        if (childNode.name() == "tileset")
        {
            Tileset *tileset = readTileset(childNode, pathDir, map);
            if (tileset)
            {
                map->addTileset(tileset);
            }
        }
        else if (childNode.name() == "layer")
        {
            readLayer(childNode, map);
        }
        else if (childNode.name() == "properties")
        {
            readProperties(childNode, map);
        }
        else if (childNode.name() == "objectgroup")
        {
            // The object group offset is applied to each object individually
            const int tileOffsetX = childNode.getProperty("x", 0);
            const int tileOffsetY = childNode.getProperty("y", 0);
            const int offsetX = tileOffsetX * tilew;
            const int offsetY = tileOffsetY * tileh;

            for (auto objectNode : childNode.children())
            {
                if (objectNode.name() == "object")
                {
                    std::string objType = objectNode.getProperty("type", "");
                    objType = toUpper(objType);

                    if (objType == "NPC" ||
                        objType == "SCRIPT" ||
                        objType == "SPAWN")
                    {
                        // Silently skip server-side objects.
                        continue;
                    }

                    const std::string objName = objectNode.getProperty("name", "");
                    const int objX = objectNode.getProperty("x", 0);
                    const int objY = objectNode.getProperty("y", 0);
                    const int objW = objectNode.getProperty("width", 0);
                    const int objH = objectNode.getProperty("height", 0);

                    logger->log("- Loading object name: %s type: %s at %d:%d",
                                objName.c_str(), objType.c_str(),
                                objX, objY);

                    if (objType == "PARTICLE_EFFECT")
                    {
                        if (objName.empty())
                        {
                            logger->log("   Warning: No particle file given");
                            continue;
                        }

                        map->addParticleEffect(objName,
                                               objX + offsetX,
                                               objY + offsetY,
                                               objW, objH);
                    }
                    else if (objType == "WARP")
                    {
                        if (config.getValue("showWarps", 1))
                        {
                            map->addParticleEffect(
                                     paths.getStringValue("particles")
                                     + paths.getStringValue("portalEffectFile"),
                                                   objX, objY, objW, objH);
                        }
                    }
                    else
                    {
                        logger->log("   Warning: Unknown object type");
                    }
                }
            }
        }
    }

    map->initializeAmbientLayers();

    return map;
}

/**
 * Reads the properties element.
 *
 * @param node  The <code>properties</code> element.
 * @param props The Properties instance to which the properties will
 *              be assigned.
 */
static void readProperties(XML::Node node, Properties *props)
{
    for (auto childNode : node.children())
    {
        if (childNode.name() != "property")
            continue;

        // Example: <property name="name" value="value"/>
        const std::string name = childNode.getProperty("name", "");
        const std::string value = childNode.getProperty("value", "");

        if (!name.empty() && !value.empty())
            props->setProperty(name, value);
    }
}

static void setTile(Map *map, MapLayer *layer, int x, int y, unsigned gid)
{
    // Bits on the far end of the 32-bit global tile ID are used for tile flags
    const int FlippedHorizontallyFlag   = 0x80000000;
    const int FlippedVerticallyFlag     = 0x40000000;
    const int FlippedAntiDiagonallyFlag = 0x20000000;

    // Clear the flags
    // TODO: It would be nice to properly support these flags later
    gid &= ~(FlippedHorizontallyFlag |
             FlippedVerticallyFlag |
             FlippedAntiDiagonallyFlag);

    const Tileset * const set = map->getTilesetWithGid(gid);
    if (layer)
    {
        // Set regular tile on a layer
        Image * const img = set ? set->get(gid - set->getFirstGid()) : nullptr;
        layer->setTile(x, y, img);

        if (TileAnimation *ani = map->getAnimationForGid(gid))
            ani->addAffectedTile(layer, x + y * layer->getWidth());
    }
    else
    {
        // Set collision tile
        if (set && (gid - set->getFirstGid() == 1))
            map->blockTile(x, y, Map::BLOCKTYPE_WALL);
    }
}

/**
 * Reads a map layer and adds it to the given map.
 */
static void readLayer(XML::Node node, Map *map)
{
    // Layers are not necessarily the same size as the map
    const int w = node.getProperty("width", map->getWidth());
    const int h = node.getProperty("height", map->getHeight());
    const int offsetX = node.getProperty("x", 0);
    const int offsetY = node.getProperty("y", 0);
    std::string name = node.getProperty("name", "");
    name = toLower(name);

    const bool isFringeLayer = (name.substr(0,6) == "fringe");
    const bool isCollisionLayer = (name.substr(0,9) == "collision");

    MapLayer *layer = nullptr;

    if (!isCollisionLayer)
    {
        layer = new MapLayer(offsetX, offsetY, w, h, isFringeLayer, map);
        map->addLayer(layer);
    }

    logger->log("- Loading layer \"%s\"", name.c_str());
    int x = 0;
    int y = 0;

    // Load the tile data
    for (auto childNode : node.children())
    {
        if (childNode.name() == "properties")
        {
            for (auto prop : childNode.children())
            {
                if (prop.name() != "property")
                    continue;

                const std::string pname = prop.getProperty("name", "");
                const std::string value = prop.getProperty("value", "");

                // TODO: Consider supporting "Hidden", "Version" and "NotVersion"

                if (pname == "Mask")
                {
                    layer->setMask(atoi(value.c_str()));
                }
            }
            continue;
        }

        if (childNode.name() != "data")
            continue;

        const std::string encoding =
            childNode.getProperty("encoding", "");
        const std::string compression =
            childNode.getProperty("compression", "");

        if (encoding == "base64")
        {
            if (!compression.empty() && compression != "gzip"
                && compression != "zlib")
            {
                logger->log("Warning: only gzip or zlib layer "
                            "compression supported!");
                return;
            }

            // Read base64 encoded map file
            const auto data = childNode.textContent();
            if (data.empty())
                continue;

            auto *charStart = data.data();
            auto *charData = new unsigned char[data.length() + 1];
            unsigned char *charIndex = charData;

            while (*charStart)
            {
                if (*charStart != ' ' &&
                    *charStart != '\t' &&
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
                php3_base64_decode(charData,
                                   charIndex - charData,
                                   &binLen);

            delete[] charData;

            if (binData)
            {
                if (compression == "gzip" || compression == "zlib")
                {
                    // Inflate the gzipped layer data
                    unsigned char *inflated;
                    unsigned int inflatedSize =
                        inflateMemory(binData, binLen, inflated);

                    free(binData);
                    binData = inflated;
                    binLen = inflatedSize;

                    if (!inflated)
                    {
                        logger->log("Error: Could not decompress layer!");
                        return;
                    }
                }

                for (int i = 0; i < binLen - 3; i += 4)
                {
                    const unsigned gid = binData[i] |
                        binData[i + 1] << 8 |
                        binData[i + 2] << 16 |
                        binData[i + 3] << 24;

                    setTile(map, layer, x, y, gid);

                    x++;
                    if (x == w)
                    {
                        x = 0; y++;

                        // When we're done, don't crash on too much data
                        if (y == h)
                            break;
                    }
                }
                free(binData);
            }
        }
        else if (encoding == "csv")
        {
            const auto data = childNode.textContent();
            if (data.empty())
            {
                logger->log("Error: CSV layer data is empty!");
                continue;
            }

            auto *pos = data.data();

            for (;;)
            {
                // Try to parse the next number at 'pos'
                errno = 0;
                char *end;
                unsigned gid = strtol(pos, &end, 10);
                if (pos == end) // No number found
                    break;

                if (errno == ERANGE)
                {
                    logger->log("Error: Range error in tile layer data!");
                    break;
                }

                setTile(map, layer, x, y, gid);

                x++;
                if (x == w)
                {
                    x = 0; y++;

                    // When we're done, don't crash on too much data
                    if (y == h)
                        break;
                }

                // Skip the comma, or break if we're done
                pos = strchr(end, ',');
                if (!pos)
                {
                    logger->log("Error: CSV layer data too short!");
                    break;
                }
                ++pos;
            }
        }
        else
        {
            // Read plain XML map file
            for (auto childNode2 : childNode.children())
            {
                if (childNode2.name() != "tile")
                    continue;

                unsigned gid = childNode2.getProperty("gid", 0);
                setTile(map, layer, x, y, gid);

                x++;
                if (x == w)
                {
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

/**
 * Reads a tile set.
 */
static Tileset *readTileset(XML::Node node, const std::string &path,
                            Map *map)
{
    const unsigned firstGid = node.getProperty("firstgid", 0);
    const int margin = node.getProperty("margin", 0);
    const int spacing = node.getProperty("spacing", 0);
    XML::Document *doc = nullptr;
    Tileset *set = nullptr;
    std::string pathDir(path);

    if (node.hasProperty("source"))
    {
        std::string filename = node.getProperty("source", std::string());
        filename = resolveRelativePath(path, filename);

        doc = new XML::Document(filename);
        node = doc->rootNode();

        // Reset path to be realtive to the tsx file
        pathDir = filename.substr(0, filename.rfind("/") + 1);
    }

    const int tw = node.getProperty("tilewidth", map->getTileWidth());
    const int th = node.getProperty("tileheight", map->getTileHeight());

    for (auto childNode : node.children())
    {
        if (childNode.name() == "image")
        {
            const auto source = childNode.getProperty("source", std::string());
            if (!source.empty())
            {
                std::string sourceStr = resolveRelativePath(pathDir, source);

                ResourceManager *resman = ResourceManager::getInstance();
                auto tilebmp = resman->getImageRef(sourceStr);

                if (tilebmp)
                {
                    set = new Tileset(tilebmp, tw, th, firstGid, margin,
                                      spacing);
                }
                else
                {
                    logger->log("Warning: Failed to load tileset (%s)",
                            source.c_str());
                }
            }
        }
        else if (set && childNode.name() == "tile")
        {
            const int tileGID = firstGid + childNode.getProperty("id", 0);

            for (auto tileNode : childNode.children())
            {
                if (tileNode.name() == "animation")
                    readTileAnimation(tileNode, set, tileGID, map);
            }
        }
    }

    delete doc;

    return set;
}

static void readTileAnimation(XML::Node tileNode,
                              Tileset *set,
                              unsigned tileGID,
                              Map *map)
{
    Animation ani;
    for (auto frameNode : tileNode.children())
    {
        if (frameNode.name() == "frame")
        {
            const int tileId = frameNode.getProperty("tileid", 0);
            const int duration = frameNode.getProperty("duration", 0);
            ani.addFrame(set->get(tileId), duration, 0, 0);
        }
    }

    if (ani.getLength() > 0)
        map->addAnimation(tileGID, TileAnimation(std::move(ani)));
}
