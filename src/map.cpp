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

#include "main.h"
#include "map.h"
#include "log.h"
#include "resources/resourcemanager.h"
#include "resources/mapreader.h"
#include "graphic/spriteset.h"
#include "net/network.h"

#include <queue>

#define OLD_MAP_WIDTH  200
#define OLD_MAP_HEIGHT 200

/**
 * Old tile structure. Used for loading the old map format.
 */
struct TILE {
    /**
     * Data field filled at follows:
     *
     * <pre>
     *  1st byte: [1][1][1][1][1][1][1][1]
     *  2nd byte: [1][1][2][2][2][2][2][2]
     *  3rd byte: [2][2][2][2][3][3][3][3]
     *  4th byte: [3][3][3][3][3][3][W][A]
     * </pre>
     *
     * Legend:
     *  1 - Ground layer (grass, water, ...)
     *  2 - Fringe layer (decoration on top of ground layer, but below beings)
     *  3 - Over layer (roofs, tree leaves, ...)
     *  W - Walkability flag
     *  A - Animated tile flag
     */
    char data[4];
    char flags;
};

/**
 * Old map structure. Used for loading the old map format.
 */
struct MAP {
    TILE tiles[OLD_MAP_WIDTH][OLD_MAP_HEIGHT];
    char tileset[20];
    char bg_music[20];
};


MetaTile::MetaTile():
    whichList(0)
{
}


Location::Location(int x, int y, MetaTile *tile):
    x(x), y(y), tile(tile)
{
}

bool Location::operator< (const Location &loc) const
{
   return tile->Fcost > loc.tile->Fcost; 
}


Map::Map():
    width(0), height(0),
    tileWidth(32), tileHeight(32),
    onClosedList(1), onOpenList(2)
{
    metaTiles = new MetaTile[width * height];
    tiles = new Image*[width * height * 3];
}

Map::Map(int width, int height):
    width(width), height(height),
    tileWidth(32), tileHeight(32),
    onClosedList(1), onOpenList(2)
{
    metaTiles = new MetaTile[width * height];
    tiles = new Image*[width * height * 3];
}

Map::~Map()
{
    delete[] metaTiles;
    delete[] tiles;
}

Map *Map::load(const std::string &mapFile)
{
    logger.log("Map::load(%s)", mapFile.c_str());

    if (mapFile.find(".tmx", 0) != std::string::npos)
    {
        // New map file format assumed
        return MapReader::readMap(mapFile);
    }

    FILE *file = fopen(mapFile.c_str(), "r");

    if (!file) {
        logger.log("Warning: %s", mapFile.c_str());
        return NULL;
    }

    MAP oldMap;
    fread(&oldMap, sizeof(MAP), 1, file);
    fclose(file);

    Map *map = new Map(OLD_MAP_WIDTH, OLD_MAP_HEIGHT);

    // Load the default tileset
    ResourceManager *resman = ResourceManager::getInstance();
    Image *tilesetbmp = resman->getImage("graphics/tiles/desert.png");
    if (!tilesetbmp) logger.error("Unable to load desert.png");
    Spriteset *tileset = new Spriteset(tilesetbmp, 32, 32);

    // Transfer tile data
    int x, y, a;
    for (y = 0; y < OLD_MAP_HEIGHT; y++) {
        for (x = 0; x < OLD_MAP_WIDTH; x++) {
            unsigned short id = 0;

            for (a = 0; a < 3; a++) {
                // Different interpretation for each layer
                switch (a) {
                    case 0:
                        id = MAKEWORD(oldMap.tiles[x][y].data[1] & 0x00c0,
                                oldMap.tiles[x][y].data[0]);
                        id >>= 6;
                        break;
                    case 1:
                        id = MAKEWORD(oldMap.tiles[x][y].data[2] & 0x00f0,
                                oldMap.tiles[x][y].data[1] & 0x003f);
                        id >>= 4;
                        break;
                    case 2:
                        id = MAKEWORD(oldMap.tiles[x][y].data[3] & 0x00fc,
                                oldMap.tiles[x][y].data[2] & 0x000f);
                        id >>= 2;
                        break;
                }

                if (id < tileset->spriteset.size() && (a == 0 || id > 0)) {
                    map->setTile(x, y, a, tileset->spriteset[id]);
                }
                else {
                    map->setTile(x, y, a, NULL);
                }
            }

            // Walkability
            map->setWalk(x, y, (oldMap.tiles[x][y].data[3] & 0x0002) > 0);
        }
    }

    return map;
}

void Map::setSize(int width, int height)
{
    this->width = width;
    this->height = height;
    delete[] metaTiles;
    delete[] tiles;
    metaTiles = new MetaTile[width * height];
    tiles = new Image*[width * height * 3];
}

void Map::draw(Graphics *graphics, int scrollX, int scrollY, int layer)
{
    int startX = scrollX / 32;
    int startY = scrollY / 32;
    int endX = (graphics->getWidth() + scrollX + 31) / 32;
    int endY = (graphics->getHeight() + scrollY + 31) / 32;

    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX >= width) endX = width - 1;
    if (endY >= height) endY = height - 1;

    for (int y = startY; y < endY; y++)
    {
        for (int x = startX; x < endX; x++)
        {
            Image *img = getTile(x, y, layer);
            if (img) {
                img->draw(screen, x * 32 - scrollX, y * 32 - scrollY);
            }
        }
    }
}

void Map::setWalk(int x, int y, bool walkable)
{
    metaTiles[x + y * width].walkable = walkable;
}

bool Map::getWalk(int x, int y)
{
    // You can't walk outside of the map
    if (x < 0 || y < 0 || x >= width || y >= height) {
        return false;
    }

    // Check if the tile is walkable
    bool ret = metaTiles[x + y * width].walkable;

    // If walkable, check for colliding into a being
    if (ret) {
        std::list<Being*>::iterator i = beings.begin();
        while (i != beings.end() && ret) {
            Being *being = (*i);
            if (being->x == x && being->y == y) {
                return false;
            }
            i++;
        }
    }

    return ret;
}

void Map::setTile(int x, int y, int layer, Image *img)
{
    tiles[x + y * width + layer * (width * height)] = img;
}

Image *Map::getTile(int x, int y, int layer)
{
    return tiles[x + y * width + layer * (width * height)];
}

MetaTile *Map::getMetaTile(int x, int y)
{
    return &metaTiles[x + y * width];
}

int Map::getWidth()
{
    return width;
}

int Map::getHeight()
{
    return height;
}

int Map::getTileWidth()
{
    return tileWidth;
}

int Map::getTileHeight()
{
    return tileHeight;
}

std::list<PATH_NODE> Map::findPath(
        int startX, int startY, int destX, int destY)
{
    // Path to be built up (empty by default)
    std::list<PATH_NODE> path;

    // Declare open list, a list with open tiles sorted on F cost
    std::priority_queue<Location> openList;

    // Return when destination not walkable
    if (!getWalk(destX, destY)) return path;

    // Reset starting tile's G cost to 0
    MetaTile *startTile = getMetaTile(startX, startY);
    startTile->Gcost = 0;

    // Add the start point to the open list
    openList.push(Location(startX, startY, startTile));

    bool foundPath = false;

    // Keep trying new open tiles until no more tiles to try or target found
    while (!openList.empty() && !foundPath)
    {
        // Take the location with the lowest F cost from the open list, and
        // add it to the closed list.
        Location curr = openList.top();
        openList.pop();

        // If the tile is already on the closed list, this means it has already
        // been processed with a shorter path to the start point (lower G cost)
        if (curr.tile->whichList == onClosedList)
        {
            continue;
        }

        // Put the current tile on the closed list
        curr.tile->whichList = onClosedList;

        // Check the adjacent tiles
        for (int dy = -1; dy <= 1; dy++)
        {
            for (int dx = -1; dx <= 1; dx++)
            {
                // Calculate location of tile to check
                int x = curr.x + dx;
                int y = curr.y + dy;

                // Skip if if we're checking the same tile we're leaving from,
                // or if the new location falls outside of the map boundaries
                if ((dx == 0 && dy == 0) ||
                        (x < 0 || y < 0 || x >= width || y >= height))
                {
                    continue;
                }

                MetaTile *newTile = getMetaTile(x, y);

                // Skip if the tile is on the closed list or is not walkable
                if (newTile->whichList == onClosedList || !getWalk(x, y))
                {
                    continue;
                }

                // When taking a diagonal step, verify that we can skip the
                // corner. We allow skipping past beings but not past non-
                // walkable tiles.
                if (dx != 0 && dy != 0)
                {
                    MetaTile *t1 = getMetaTile(curr.x, curr.y + dy);
                    MetaTile *t2 = getMetaTile(curr.x + dx, curr.y);

                    if (!(t1->walkable && t2->walkable))
                    {
                        continue;
                    }
                }

                // Calculate G cost for this route, 10 for moving straight and
                // 14 for moving diagonal
                int Gcost = curr.tile->Gcost + ((dx == 0 || dy == 0) ? 10 : 14);

                if (newTile->whichList != onOpenList)
                {
                    // Found a new tile (not on open nor on closed list)
                    // Update Hcost of the new tile using Manhatten distance
                    newTile->Hcost = 10 * (abs(x - destX) + abs(y - destY));

                    // Set the current tile as the parent of the new tile
                    newTile->parentX = curr.x;
                    newTile->parentY = curr.y;

                    // Update Gcost and Fcost of new tile
                    newTile->Gcost = Gcost;
                    newTile->Fcost = newTile->Gcost + newTile->Hcost;

                    if (x != destX || y != destY) {
                        // Add this tile to the open list
                        newTile->whichList = onOpenList;
                        openList.push(Location(x, y, newTile));
                    }
                    else {
                        // Target location was found
                        foundPath = true;
                    }
                }
                else if (Gcost < newTile->Gcost)
                {
                    // Found a shorter route.
                    // Update Gcost and Fcost of the new tile
                    newTile->Gcost = Gcost;
                    newTile->Fcost = newTile->Gcost + newTile->Hcost;

                    // Set the current tile as the parent of the new tile
                    newTile->parentX = curr.x;
                    newTile->parentY = curr.y;

                    // Add this tile to the open list (it's already
                    // there, but this instance has a lower F score)
                    openList.push(Location(x, y, newTile));
                }
            }
        }
    }

    // Two new values to indicate wether a tile is on the open or closed list,
    // this way we don't have to clear all the values between each pathfinding.
    onClosedList += 2;
    onOpenList += 2;

    // If a path has been found, iterate backwards using the parent locations
    // to extract it.
    if (foundPath)
    {
        int pathX = destX;
        int pathY = destY;

        while (pathX != startX || pathY != startY)
        {
            // Add the new path node to the start of the path list
            path.push_front(PATH_NODE(pathX, pathY));

            // Find out the next parent
            MetaTile *tile = getMetaTile(pathX, pathY);
            pathX = tile->parentX;
            pathY = tile->parentY;
        }
    }

    return path;
}
