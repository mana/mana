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
#include "being.h"

#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#else
#include "./net/win2linux.h"
#endif

Map tiledMap;

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


Tile::Tile():
    whichList(0)
{
}


bool Map::load(char *mapFile) {
    FILE *file = fopen(mapFile, "r");

    if (!file) {
        warning(mapFile);
        return false;
    }

    MAP oldMap;
    fread(&oldMap, sizeof(MAP), 1, file);
    fclose(file);

    // Transfer tile data
    int x, y;
    for (y = 0; y < OLD_MAP_HEIGHT; y++) {
        for (x = 0; x < OLD_MAP_WIDTH; x++) {
            unsigned short id;

            // Layer 0
            id = MAKEWORD(oldMap.tiles[x][y].data[1] & 0x00c0,
                    oldMap.tiles[x][y].data[0]);
            id >>= 6;
            setTile(x, y, 0, id);

            // Layer 1
            id = MAKEWORD(oldMap.tiles[x][y].data[2] & 0x00f0,
                    oldMap.tiles[x][y].data[1] & 0x003f);
            id >>= 4;
            setTile(x, y, 1, id);

            // Layer 2
            id = MAKEWORD(oldMap.tiles[x][y].data[3] & 0x00fc,
                    oldMap.tiles[x][y].data[2] & 0x000f);
            id >>= 2;
            setTile(x, y, 2, id);

            // Walkability
            setWalk(x, y, (oldMap.tiles[x][y].data[3] & 0x0002) > 0);
        }
    }

    return true;
}

void Map::setWalk(int x, int y, bool walkable) {
    if (walkable) {
        tiles[x][y].flags |= TILE_WALKABLE;
    }
    else {
        tiles[x][y].flags &= ~TILE_WALKABLE;
    }
}

bool Map::getWalk(int x, int y) {
    bool ret = (tiles[x][y].flags & TILE_WALKABLE) != 0;

    if (ret) {
        // Check for colliding into a being
        std::list<Being*>::iterator i = beings.begin();
        while (i != beings.end() && ret) {
            Being *being = (*i);
            if (being->x == x && being->y == y) {
                ret = false;
            }
            i++;
        }
    }

    return ret;
}

void Map::setTile(int x, int y, int layer, int id)
{
    tiles[x][y].layers[layer] = id;
}

int Map::getTile(int x, int y, int layer)
{
    return tiles[x][y].layers[layer];
}

int Map::getWidth()
{
    return width;
}

int Map::getHeight()
{
    return height;
}
