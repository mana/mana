/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MAP_H
#define MAP_H

#include <list>
#include <vector>

#include "position.h"
#include "properties.h"

class Animation;
class AmbientOverlay;
class Graphics;
class Image;
class MapLayer;
class Particle;
class SimpleAnimation;
class Sprite;
class Tileset;

typedef std::vector<Tileset*> Tilesets;
typedef std::list<Sprite*> Sprites;
typedef Sprites::iterator SpriteIterator;
typedef std::vector<MapLayer*> Layers;

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
    MetaTile() : whichList(0), blockmask(0) {}

    // Pathfinding members
    int Fcost;               /**< Estimation of total path cost */
    int Gcost;               /**< Cost from start to this location */
    int Hcost;               /**< Estimated cost to goal */
    int whichList;           /**< No list, open list or closed list */
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
         * fringe layer. The fringe layer is the layer that draws the sprites.
         * There can be only one fringe layer per map.
         */
        MapLayer(int x, int y, int width, int height, bool isFringeLayer);

        /**
         * Destructor.
         */
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
        Image *getTile(int x, int y) const;

        /**
         * Draws this layer to the given graphics context. The coordinates are
         * expected to be in map range and will be translated to local layer
         * coordinates and clipped to the layer's dimensions.
         *
         * The given sprites are only drawn when this layer is the fringe
         * layer.
         */
        void draw(Graphics *graphics,
                  int startX, int startY,
                  int endX, int endY,
                  int scrollX, int scrollY,
                  const Sprites &sprites) const;

    private:
        int mX, mY;
        int mWidth, mHeight;
        bool mIsFringeLayer;    /**< Whether the sprites are drawn. */
        Image **mTiles;
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
         * Updates animations. Called as needed.
         */
        void update(int ticks = 1);

        /**
         * Draws the map to the given graphics output. This method draws all
         * layers, sprites and overlay effects.
         *
         * TODO: For efficiency reasons, this method could take into account
         * the clipping rectangle set on the Graphics object. However,
         * currently the map is always drawn full-screen.
         */
        void draw(Graphics *graphics, int scrollX, int scrollY);

        /**
         * Visualizes collision layer for debugging
         */
        void drawCollision(Graphics *graphics, int scrollX, int scrollY);

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
        Tileset *getTilesetWithGid(int gid) const;

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

#ifdef EATHENA_SUPPORT
        /**
         * Tells whether a tile is occupied by a being.
         */
        bool occupied(int x, int y) const;
#endif

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
        int getTileWidth() const { return mTileWidth; }

        /**
         * Returns the tile height used by this map.
         */
        int getTileHeight() const { return mTileHeight; }

        const std::string &getMusicFile() const;
        const std::string &getName() const;

        /**
         * Find a simple path from one location to the next.
         */
        Path findSimplePath(int startX, int startY,
                            int destX, int destY,
                            unsigned char walkmask);

        /**
         * Find a path from one location to the next.
         */
        Path findPath(int startX, int startY, int destX, int destY,
                      unsigned char walkmask, int maxCost = 20);

        /**
         * Adds a sprite to the map.
         */
        SpriteIterator addSprite(Sprite *sprite);

        /**
         * Removes a sprite from the map.
         */
        void removeSprite(SpriteIterator iterator);

        /**
         * Adds a particle effect
         */
        void addParticleEffect(const std::string &effectFile, int x, int y);

        /**
         * Initializes all added particle effects
         */
        void initializeParticleEffects(Particle* particleEngine);

        /**
         * Adds a tile animation to the map
         */
        void addAnimation(int gid, TileAnimation *animation)
        { mTileAnimations[gid] = animation; }

        /**
         * Gets the tile animation for a specific gid
         */
        TileAnimation *getAnimationForGid(int gid);

    private:
        /**
         * Draws the overlay graphic to the given graphics output.
         */
        void drawOverlay(Graphics *graphics, float scrollX, float scrollY,
                         int detail);

        /**
         * Tells whether the given coordinates fall within the map boundaries.
         */
        bool contains(int x, int y) const;

        /**
         * Blockmasks for different entities
         */
        int *mOccupation[NB_BLOCKTYPES];

        int mWidth, mHeight;
        int mTileWidth, mTileHeight;
        int mMaxTileHeight;
        MetaTile *mMetaTiles;
        Layers mLayers;
        Tilesets mTilesets;
        Sprites mSprites;

        // Pathfinding members
        int mOnClosedList, mOnOpenList;

        // Overlay data
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

        std::map<int, TileAnimation*> mTileAnimations;
};

#endif
