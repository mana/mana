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

#ifndef _TMW_MAP_H
#define _TMW_MAP_H

#define TILESET_WIDTH 30

#define WALKABLE 0
#define NOT_WALKABLE 1

/** Struct representing a tile. A tile is composed of 3 layers.
1st: Ground layer (used for grass, water, ...) -> bit 0-9 of data
2nd: Fringe layer (objects that are overlapped by the player) -> bit 10-19 of data
3rd: Over layer (roofs, tree leaves, ...) -> bit 20-29
Walk flag: tells if a tile is walkable or not -> bit 30
Animation flag: tells if a tile is animated or not -> bit 31

data field:
1st byte: [1][1][1][1][1][1][1][1]
2nd byte: [1][1][2][2][2][2][2][2]
3rd byte: [2][2][2][2][3][3][3][3]
4th byte: [3][3][3][3][3][3][W][A]

Legend:
1 - First layer
2 - Second layer
3 - Third layer
W - Walkability flag
A - Animated tile flag

flags field:
[l][l][f][f][f][f][f][f][f]

Legend:
l - Animated layer
f - future use
*/
struct TILE {
    char data[4];
    char flags;
};

class Map
{
    public:
        /**
         * Loads a map file
         */
        bool load(char *mapFile);

        /**
         * Set tile ID.
         */
        void setTile(int x, int y, int layer, unsigned short id);

        /**
         * Get tile ID.
         */
        int getTile(int x, int y, int layer);

        /**
         * Set walkability flag for a tile
         */
        void setWalk(int x, int y, bool walkable);

        /**
         * Tell if a tile is walkable or not
         */
        bool getWalk(int x, int y);

        /**
         * Tell if a tile is walkable or not (0=walkable,1=not walkable)
         */
        int getPathWalk(int x, int y);

        /**
         * Returns the width of this map.
         */
        int getWidth();

        /**
         * Returns the height of this map.
         */
        int getHeight();

    private:
        const static int width = 200;
        const static int height = 200;
        TILE tiles[width][height];
        char tileset[20];
        char bg_music[20];
};

extern Map tiledMap;

#endif
