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
 */

#ifndef _TMW_MAP_H_
#define _TMW_MAP_H_

#include <list>
#include <vector>

#include "position.h"
#include "properties.h"

#include "simpleanimation.h"

class AmbientOverlay;
class Graphics;
class Image;
class MapLayer;
class Particle;
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
    MetaTile():whichList(0) {};

    // Pathfinding members
    int Fcost;               /**< Estimation of total path cost */
    int Gcost;               /**< Cost from start to this location */
    int Hcost;               /**< Estimated cost to goal */
    int whichList;           /**< No list, open list or closed list */
    int parentX;             /**< X coordinate of parent tile */
    int parentY;             /**< Y coordinate of parent tile */
    bool walkable;           /**< Can beings walk on this tile */
};

/**
 * Animation cycle of a tile image which changes the map accordingly.
 */
class TileAnimation
{
    public:
        TileAnimation(Animation *ani);
        void update();
        void addAffectedTile(MapLayer *layer, int index)
        { mAffected.push_back(std::make_pair(layer, index)); }
    private:
        std::list<std::pair<MapLayer*, int> > mAffected;
        SimpleAnimation mAnimation;
        int mLastUpdate;
        Image* mLastImage;
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
         * Draws the map to the given graphics output. This method draws all
         * layers, sprites and overlay effects.
         *
         * TODO: For efficiency reasons, this method could take into account
         * the clipping rectangle set on the Graphics object. However,
         * currently the map is always drawn full-screen.
         */
        void draw(Graphics *graphics, int scrollX, int scrollY);

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
        Tileset* getTilesetWithGid(int gid) const;

        /**
         * Get tile reference.
         */
        MetaTile *getMetaTile(int x, int y);

        /**
         * Set walkability flag for a tile.
         */
        void setWalk(int x, int y, bool walkable);

        /**
         * Tell if a tile collides, not including a check on beings.
         */
        bool tileCollides(int x, int y) const;

        /**
         * Returns the width of this map.
         */
        int getWidth() const { return mWidth; }

        /**
         * Returns the height of this map.
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

        /**
         * Find a path from one location to the next.
         */
        Path findPath(int startX, int startY, int destX, int destY);

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
         * Tells whether a tile is occupied by a being.
         */
        bool occupied(int x, int y) const;

        /**
         * Tells whether the given coordinates fall within the map boundaries.
         */
        bool contains(int x, int y) const;

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
