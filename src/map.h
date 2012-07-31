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

#ifndef MAP_H
#define MAP_H

#include "actor.h"
#include "position.h"
#include "properties.h"

#include <list>
#include <vector>

class Animation;
class AmbientLayer;
class Graphics;
class MapLayer;
class Particle;
class SimpleAnimation;
class Tileset;

typedef std::vector<Tileset*> Tilesets;
typedef std::vector<MapLayer*> Layers;

const int DEFAULT_TILE_LENGTH = 32;

/**
 * A meta tile stores additional information about a location on a tile map.
 * This is information that doesn't need to be repeated for each tile in each
 * layer of the map.
 */
struct MetaTile
{
    MetaTile() : whichList(0), blockmask(0) {}

    // Pathfinding members
    int Fcost;               /**< Estimation of total path cost */
    int Gcost;               /**< Cost from start to this location */
    int Hcost;               /**< Estimated cost to goal */
    unsigned whichList;      /**< No list, open list or closed list */
    int parentX;             /**< X coordinate of parent tile */
    int parentY;             /**< Y coordinate of parent tile */
    unsigned char blockmask; /**< Blocking properties of this tile */
};

/**
 * Animation cycle of a tile image which changes the map accordingly.
 */
class TileAnimation
{
    public:
        TileAnimation(Animation *ani);
        ~TileAnimation();
        void update(int ticks = 1);
        void addAffectedTile(MapLayer *layer, int index)
        { mAffected.push_back(std::make_pair(layer, index)); }
    private:
        std::list<std::pair<MapLayer*, int> > mAffected;
        SimpleAnimation *mAnimation;
        Image *mLastImage;
};

/**
 * A map layer. Stores a grid of tiles and their offset, and implements layer
 * rendering.
 */
class MapLayer
{
    public:
        /**
         * Constructor, taking layer origin, size and whether this layer is the
         * fringe layer. The fringe layer is the layer that draws the actors.
         * There can be only one fringe layer per map.
         */
        MapLayer(int x, int y, int width, int height, bool isFringeLayer,
                 Map *map);

        ~MapLayer();

        /**
         * Set tile image, with x and y in layer coordinates.
         */
        void setTile(int x, int y, Image *img);

        /**
         * Set tile image with x + y * width already known.
         */
        void setTile(int index, Image *img) { mTiles[index] = img; }

        /**
         * Get tile image, with x and y in layer coordinates.
         */
        Image *getTile(int x, int y) const
        { return mTiles[x + y * mWidth]; }

        /**
         * Draws this layer to the given graphics context. The coordinates are
         * expected to be in map range and will be translated to local layer
         * coordinates and clipped to the layer's dimensions.
         *
         * The given actors are only drawn when this layer is the fringe
         * layer.
         */
        void draw(Graphics *graphics,
                  int startX, int startY,
                  int endX, int endY,
                  int scrollX, int scrollY,
                  const Actors &actors,
                  int debugFlags) const;

        bool isFringeLayer() const
        { return mIsFringeLayer; }

        int getTileDrawWidth(int x1, int y1, int endX, int &width) const;

    private:
        int mX, mY;
        int mWidth, mHeight;
        bool mIsFringeLayer;    /**< Whether the actors are drawn. */
        Image **mTiles;
        Map *mMap;              /** The mother map pointer */
};

/**
 * A tile map.
 */
class Map : public Properties
{
    public:
        enum BlockType
        {
            BLOCKTYPE_NONE = -1,
            BLOCKTYPE_WALL,
            BLOCKTYPE_CHARACTER,
            BLOCKTYPE_MONSTER,
            NB_BLOCKTYPES
        };

        enum BlockMask
        {
            BLOCKMASK_WALL      = 0x80, // = bin 1000 0000
            BLOCKMASK_CHARACTER = 0x01, // = bin 0000 0001
            BLOCKMASK_MONSTER   = 0x02  // = bin 0000 0010
        };

        enum DebugFlags
        {
            DEBUG_NONE = 0x0,
            DEBUG_GRID = 0x1,
            DEBUG_COLLISION_TILES = 0x2,
            DEBUG_BEING_COLLISION_RADIUS = 0x4,
            DEBUG_BEING_POSITION = 0x8,
            DEBUG_BEING_PATH = 0x10,
            DEBUG_MOUSE_PATH = 0x20,
            DEBUG_SPECIAL1 = 0x40,
            DEBUG_SPECIAL2 = 0x80,
            DEBUG_SPECIAL3 = 0x100
        };

        /**
         * Constructor, taking map and tile size as parameters.
         */
        Map(int width, int height, int tileWidth, int tileHeight);

        ~Map();

        /**
         * Initialize ambient layers. Has to be called after all the properties
         * are set.
         */
        void initializeAmbientLayers();

        /**
         * Updates animations. Called as needed.
         */
        void update(int ticks = 1);

        /**
         * Draws the map to the given graphics output. This method draws all
         * layers, actors and overlay effects.
         *
         * TODO: For efficiency reasons, this method could take into account
         * the clipping rectangle set on the Graphics object. However,
         * currently the map is always drawn full-screen.
         */
        void draw(Graphics *graphics, int scrollX, int scrollY);

        /**
         * Visualizes collision layer for debugging
         */
        void drawCollision(Graphics *graphics, int scrollX, int scrollY,
                           int debugFlags);

        /**
         * Adds a layer to this map. The map takes ownership of the layer.
         */
        void addLayer(MapLayer *layer);

        /**
         * Adds a tileset to this map. The map takes ownership of the tileset.
         */
        void addTileset(Tileset *tileset);

        /**
         * Finds the tile set that a tile with the given global id is part of.
         */
        Tileset *getTilesetWithGid(unsigned gid) const;

        /**
         * Get tile reference.
         */
        MetaTile *getMetaTile(int x, int y) const;

        /**
         * Marks a tile as occupied.
         */
        void blockTile(int x, int y, BlockType type);

        /**
         * Gets walkability for a tile with a blocking bitmask. When called
         * without walkmask, only blocks against colliding tiles.
         */
        bool getWalk(int x, int y,
                     unsigned char walkmask = BLOCKMASK_WALL) const;

        /**
         * Tells whether a tile is occupied by a being.
         */
        bool occupied(int x, int y) const;

        /**
         * Returns the width of this map in tiles.
         */
        int getWidth() const { return mWidth; }

        /**
         * Returns the height of this map in tiles.
         */
        int getHeight() const { return mHeight; }

        /**
         * Returns the tile width of this map.
         */
        int getTileWidth() const
        { return mTileWidth; }

        /**
         * Returns the tile height used by this map.
         */
        int getTileHeight() const
        { return mTileHeight; }

        /**
         * Returns the tile center position in pixel coordinates.
         *
         * @param x the horizontal tile position
         * @param y the vertical tile position
         */
        Vector getTileCenter(int x, int y);

        std::string getMusicFile() const;
        std::string getName() const;

        /**
         * Gives the map id based on filepath (ex: 009-1)
         */
        std::string getFilename() const;

        /**
         * Check the current position against surrounding blocking tiles, and
         * correct the position offset within tile when needed.
         */
        Position checkNodeOffsets(int radius, unsigned char walkMask,
                                  const Position &position) const;
        Position checkNodeOffsets(int radius, unsigned char walkMask,
                                  int x, int y) const
        { return checkNodeOffsets(radius, walkMask, Position(x, y)); }

        /**
         * Find a tile-centered path in pixel coordinates
         * from one location to the next.
         */
        Path findTilePath(int startPixelX, int startPixelY, int endPixelX,
                          int endPixelY, unsigned char walkMask,
                          int maxCost = 20);

        /**
         * Find a pixel path from one location to the next using free offsets.
         */
        Path findPixelPath(int startPixelX, int startPixelY,
                          int destPixelX, int destPixelY,
                          int radius, unsigned char walkmask, int maxCost = 20);

        /**
         * Adds a particle effect
         */
        void addParticleEffect(const std::string &effectFile, int x, int y, int w = 0, int h = 0);

        /**
         * Initializes all added particle effects
         */
        void initializeParticleEffects(Particle* particleEngine);

        /**
         * Adds a tile animation to the map
         */
        void addAnimation(int gid, TileAnimation *animation)
        { mTileAnimations[gid] = animation; }

        void setDebugFlags(int flags) { mDebugFlags = flags; }

        int getDebugFlags() const { return mDebugFlags; }

        /**
         * Gets the tile animation for a specific gid
         */
        TileAnimation *getAnimationForGid(int gid) const;

    protected:
        friend class Actor;

        /**
         * Adds an actor to the map.
         */
        Actors::iterator addActor(Actor *actor);

        /**
         * Removes an actor from the map.
         */
        void removeActor(Actors::iterator iterator);

    private:
        /**
         * Find a path from one location to the next in tile coordinates.
         */
        Path findPath(int startX, int startY, int destX, int destY,
                      unsigned char walkmask, int maxCost = 20);

        enum LayerType
        {
            FOREGROUND_LAYERS,
            BACKGROUND_LAYERS
        };

        /**
         * Updates scrolling of ambient layers. Has to be called each game tick.
         */
        void updateAmbientLayers(float scrollX, float scrollY);

        /**
         * Draws the foreground or background layers to the given graphics output.
         */
        void drawAmbientLayers(Graphics *graphics, LayerType type, float scrollX, float scrollY,
                         int detail);

        /**
         * Tells whether the given coordinates fall within the map boundaries.
         */
        bool contains(int x, int y) const;

        /**
         * Blockmasks for different entities
         */
        unsigned *mOccupation[NB_BLOCKTYPES];

        int mWidth, mHeight;
        int mTileWidth, mTileHeight;
        int mMaxTileHeight, mMaxTileWidth;
        MetaTile *mMetaTiles;
        Layers mLayers;
        Tilesets mTilesets;
        Actors mActors;

        // debug flags
        int mDebugFlags;

        // Pathfinding members
        unsigned mOnClosedList, mOnOpenList;

        // Overlay data
        std::list<AmbientLayer*> mBackgrounds;
        std::list<AmbientLayer*> mForegrounds;
        float mLastScrollX;
        float mLastScrollY;

        // Particle effect data
        struct ParticleEffectData
        {
            std::string file;
            int x;
            int y;
            int w;
            int h;
        };
        std::list<ParticleEffectData> particleEffects;

        std::map<int, TileAnimation*> mTileAnimations;

};

#endif
