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

#include "being.h"

// Tile flags
#define TILE_WALKABLE  1
#define TILE_ANIMATED  2

/**
 * A tile on a tile map.
 */
class Tile
{
    public:
        /**
         * Constructor.
         */
        Tile();

        // Tile data
        int layers[3];
        char flags;

        // Pathfinding members
        int Fcost, Gcost, Hcost;
        int whichList;
        int parentX, parentY;
};

/**
 * A location on a tile map. Used for pathfinding, open list.
 */
class Location
{
    public:
        /**
         * Constructor.
         */
        Location(int x, int y, Tile *tile);

        /**
         * Comparison operator.
         */
        bool operator< (const Location &loc) const;

        int x, y;
        Tile *tile;
};

/**
 * A tile map.
 */
class Map
{
    public:
        /**
         * Constructor.
         */
        Map();

        /**
         * Destructor.
         */
        ~Map();

        /**
         * Loads a map file (gat).
         */
        bool load(const std::string &mapFile);
        
        /**
         * Loads an XML map file (tmx).
         */
        bool loadXmlMap(const std::string &mapFile);

        /**
         * Sets the size of the map. This will destroy any existing map data.
         */
        void setSize(int width, int height);

        /**
         * Set tile ID.
         */
        void setTile(int x, int y, int layer, int id);

        /**
         * Get tile ID.
         */
        int getTile(int x, int y, int layer);

        /**
         * Get tile reference.
         */
        Tile *getTile(int x, int y);

        /**
         * Set walkability flag for a tile
         */
        void setWalk(int x, int y, bool walkable);

        /**
         * Tell if a tile is walkable or not
         */
        bool getWalk(int x, int y);

        /**
         * Returns the width of this map.
         */
        int getWidth();

        /**
         * Returns the height of this map.
         */
        int getHeight();

        /**
         * Find a path from one location to the next.
         */
        PATH_NODE *findPath(int startX, int startY, int destX, int destY);

    private:
        int width, height;
        Tile *tiles;

        // Pathfinding members
        int onClosedList, onOpenList;
};

extern Map tiledMap;

#endif
