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

#ifndef _TMW_MAP_H_
#define _TMW_MAP_H_

#include <list>
#include <vector>

#include "properties.h"

class AmbientOverlay;
class Graphics;
class Image;
class Particle;
class Sprite;
class Tileset;

struct PATH_NODE;

typedef std::vector<Tileset*> Tilesets;
typedef Tilesets::iterator TilesetIterator;
typedef std::list<Sprite*> Sprites;
typedef Sprites::iterator SpriteIterator;

extern volatile int tick_time;

/**
 * A meta tile stores additional information about a location on a tile map.
 * This is information that doesn't need to be repeated for each tile in each
 * layer of the map.
 */
struct MetaTile
{
    /**
     * Constructor.
     */
    MetaTile():whichList(0) {};

    // Pathfinding members
    int Fcost;              /**< Estimation of total path cost */
    int Gcost;              /**< Cost from start to this location */
    int Hcost;              /**< Estimated cost to goal */
    int whichList;          /**< No list, open list or closed list */
    int parentX;            /**< X coordinate of parent tile */
    int parentY;            /**< Y coordinate of parent tile */
    bool walkable;          /**< Can beings walk on this tile */
};

/**
 * A tile map.
 */
class Map : public Properties
{
    public:
        /**
         * Constructor, taking map and tile size as parameters.
         */
        Map(int width, int height, int tileWidth, int tileHeight);

        /**
         * Destructor.
         */
        ~Map();

        /**
         * Initialize map overlays. Should be called after all the properties
         * are set.
         */
        void initializeOverlays();

        /**
         * Draws a map layer to the given graphics output.
         */
        void draw(Graphics *graphics, int scrollX, int scrollY, int layer);

        /**
         * Draws the overlay graphic to the given graphics output.
         */
        void
        drawOverlay(Graphics *graphics, float scrollX, float scrollY,
                    int detail);

        /**
         * Adds a tileset to this map.
         */
        void
        addTileset(Tileset *tileset);

        /**
         * Sets a tile using a global tile id. Used by the layer loading
         * routine.
         */
        void
        setTileWithGid(int x, int y, int layer, int gid);

        /**
         * Set tile ID.
         */
        void setTile(int x, int y, int layer, Image *img);

        /**
         * Get tile ID.
         */
        Image *getTile(int x, int y, int layer);

        /**
         * Get tile reference.
         */
        MetaTile *getMetaTile(int x, int y);

        /**
         * Set walkability flag for a tile.
         */
        void setWalk(int x, int y, bool walkable);

        /**
         * Tell if a tile is walkable or not, includes checking beings.
         */
        bool getWalk(int x, int y);

        /**
         * Tell if a tile collides, not including a check on beings.
         */
        bool tileCollides(int x, int y);

        /**
         * Returns the width of this map.
         */
        int
        getWidth() { return mWidth; }

        /**
         * Returns the height of this map.
         */
        int
        getHeight() { return mHeight; }

        /**
         * Returns the tile width of this map.
         */
        int
        getTileWidth() { return mTileWidth; }

        /**
         * Returns the tile height used by this map.
         */
        int
        getTileHeight() { return mTileHeight; }

        /**
         * Find a path from one location to the next.
         */
        std::list<PATH_NODE>
        findPath(int startX, int startY, int destX, int destY);

        /**
         * Adds a sprite to the map.
         */
        SpriteIterator
        addSprite(Sprite *sprite);

        /**
         * Removes a sprite from the map.
         */
        void
        removeSprite(SpriteIterator iterator);

        /**
         * Adds a particle effect
         */
        void addParticleEffect (std::string effectFile, int x, int y);

        /**
         * Initializes all added particle effects
         */
        void
        initializeParticleEffects(Particle* particleEngine);

    private:
        /**
         * Converts a global tile id to the Image* pointing to the associated
         * tile image.
         */
        Image*
        getTileWithGid(int gid);

        /**
         * Finds the tile set that a tile with the given global id is part of.
         */
        Tileset*
        getTilesetWithGid(int gid);

        /**
         * Tells whether a tile is occupied by a being.
         */
        bool occupied(int x, int y);

        /**
         * Tells whether the given coordinates fall within the map boundaries.
         */
        bool contains(int x, int y);

        int mWidth, mHeight;
        int mTileWidth, mTileHeight;
        MetaTile *mMetaTiles;
        Image **mTiles;

        Tilesets mTilesets;
        Sprites mSprites;

        // Pathfinding members
        int mOnClosedList, mOnOpenList;

        // Overlay Data
        std::list<AmbientOverlay*> mOverlays;
        float mLastScrollX;
        float mLastScrollY;

        // Particle effect data
        struct ParticleEffectData
        {
            std::string file;
            int x;
            int y;
        };
        std::list<ParticleEffectData> particleEffects;
};

#endif
