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


bool Map::load(char *mapFile) {
    FILE *file = fopen(mapFile, "r");
    if (!file) {
        warning(mapFile);
        return false;
    }
    fread(this, sizeof(Map), 1, file);
    fclose(file);
    return true;
}

void Map::setWalk(int x, int y, bool walkable) {
    if (walkable) tiles[x][y].data[3] |= 0x0002;
    else tiles[x][y].data[3] &= 0x00fd;
}

bool Map::getWalk(int x, int y) {
    bool ret = (tiles[x][y].data[3] & 0x0002) > 0;

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

void Map::setTile(int x, int y, int layer, unsigned short id) {
    if (layer == 0) {
        id <<= 6;
        tiles[x][y].data[0] = HIBYTE(id);
        tiles[x][y].data[1] &= 0x003f;
        tiles[x][y].data[1] |= LOBYTE(id);
    }
    else if (layer == 1) {
        id <<= 4;
        tiles[x][y].data[1] &= 0x00c0;
        tiles[x][y].data[1] |= HIBYTE(id);
        tiles[x][y].data[2] &= 0x000f;
        tiles[x][y].data[2] |= LOBYTE(id);
    }
    else if (layer == 2) {
        id <<= 2;
        tiles[x][y].data[2] &= 0x00f0;
        tiles[x][y].data[2] |= HIBYTE(id);
        tiles[x][y].data[3] &= 0x0003;
        tiles[x][y].data[3] |= LOBYTE(id);
    }
}

int Map::getTile(int x, int y, int layer) {
    unsigned short id = 0;
    if (layer == 0) {
        id = MAKEWORD(tiles[x][y].data[1] & 0x00c0,
                tiles[x][y].data[0]);
        id >>= 6;
    }
    else if (layer == 1) {
        id = MAKEWORD(tiles[x][y].data[2] & 0x00f0,
                tiles[x][y].data[1] & 0x003f);
        id >>= 4;
    }
    else if (layer == 2) {
        id = MAKEWORD(tiles[x][y].data[3] & 0x00fc,
                tiles[x][y].data[2] & 0x000f);
        id >>= 2;
    }
    return id;
}

int Map::getWidth()
{
    return width;
}

int Map::getHeight()
{
    return height;
}
