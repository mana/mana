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

#include <queue>

#include "beingmanager.h"
#include "configuration.h"
#include "game.h"
#include "graphics.h"
#include "map.h"
#include "particle.h"
#include "simpleanimation.h"
#include "sprite.h"
#include "tileset.h"

#include "resources/ambientoverlay.h"
#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "utils/dtor.h"
#include "utils/tostring.h"

extern volatile int tick_time;

/**
 * A location on a tile map. Used for pathfinding, open list.
 */
struct Location
{
    /**
     * Constructor.
     */
    Location(int px, int py, MetaTile *ptile):x(px),y(py),tile(ptile) {};

    /**
     * Comparison operator.
     */
    bool operator< (const Location &loc) const
    {
        return tile->Fcost > loc.tile->Fcost;
    }

    int x, y;
    MetaTile *tile;
};

TileAnimation::TileAnimation(Animation *ani):
    mLastImage(NULL)
{
    mAnimation = new SimpleAnimation(ani);
}

TileAnimation::~TileAnimation()
{
    delete mAnimation;
}

void TileAnimation::update()
{
    if (!mAnimation)
        return;

    //update animation
    mAnimation->update(1);

    // exchange images
    Image *img = mAnimation->getCurrentImage();
    if (img != mLastImage)
    {
        for (std::list<std::pair<MapLayer*, int> >::iterator i = 
             mAffected.begin(); i != mAffected.end(); i++)
        {
            i->first->setTile(i->second, img);
        }
        mLastImage = img;
    }
}

MapLayer::MapLayer(int x, int y, int width, int height, bool isFringeLayer):
    mX(x), mY(y),
    mWidth(width), mHeight(height),
    mIsFringeLayer(isFringeLayer)
{
    const int size = mWidth * mHeight;
    mTiles = new Image*[size];
    std::fill_n(mTiles, size, (Image*) 0);
}

MapLayer::~MapLayer()
{
    delete[] mTiles;
}

void MapLayer::setTile(int x, int y, Image *img)
{
    setTile(x + y * mWidth, img);
}

Image* MapLayer::getTile(int x, int y) const
{
    return mTiles[x + y * mWidth];
}

void MapLayer::draw(Graphics *graphics,
                    int startX, int startY,
                    int endX, int endY,
                    int scrollX, int scrollY,
                    const Sprites &sprites) const
{
    startX -= mX;
    startY -= mY;
    endX -= mX;
    endY -= mY;

    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX > mWidth) endX = mWidth;
    if (endY > mHeight) endY = mHeight;

    Sprites::const_iterator si = sprites.begin();

    for (int y = startY; y < endY; y++)
    {
        // If drawing the fringe layer, make sure all sprites above this row of
        // tiles have been drawn
        if (mIsFringeLayer)
        {
            while (si != sprites.end() && (*si)->getPixelY() <= y * 32 - 32)
            {
                (*si)->draw(graphics, -scrollX, -scrollY);
                si++;
            }
        }

        for (int x = startX; x < endX; x++)
        {
            Image *img = getTile(x, y);
            if (img)
            {
                const int px = (x + mX) * 32 - scrollX;
                const int py = (y + mY) * 32 - scrollY + 32 - img->getHeight();
                graphics->drawImage(img, px, py);
            }
        }
    }

    // Draw any remaining sprites
    if (mIsFringeLayer) {
        while (si != sprites.end()) {
            (*si)->draw(graphics, -scrollX, -scrollY);
            si++;
        }
    }
}

Map::Map(int width, int height, int tileWidth, int tileHeight):
    mWidth(width), mHeight(height),
    mTileWidth(tileWidth), mTileHeight(tileHeight),
    mMaxTileHeight(height),
    mOnClosedList(1), mOnOpenList(2),
    mLastScrollX(0.0f), mLastScrollY(0.0f)
{
    const int size = mWidth * mHeight;

    mMetaTiles = new MetaTile[size];
}

Map::~Map()
{
    // delete metadata, layers, tilesets and overlays
    delete[] mMetaTiles;
    delete_all(mLayers);
    delete_all(mTilesets);
    delete_all(mOverlays);
    delete_all(mTileAnimations);
}

void Map::initializeOverlays()
{
    ResourceManager *resman = ResourceManager::getInstance();

    for (int i = 0;
         hasProperty("overlay" + toString(i) + "image");
         i++)
    {
        const std::string name = "overlay" + toString(i);

        Image *img = resman->getImage(getProperty(name + "image"));
        const float speedX = getFloatProperty(name + "scrollX");
        const float speedY = getFloatProperty(name + "scrollY");
        const float parallax = getFloatProperty(name + "parallax");

        if (img)
        {
            mOverlays.push_back(
                    new AmbientOverlay(img, parallax, speedX, speedY));

            // The AmbientOverlay takes control over the image.
            img->decRef();
        }
    }
}

void Map::addLayer(MapLayer *layer)
{
    mLayers.push_back(layer);
}

void Map::addTileset(Tileset *tileset)
{
    mTilesets.push_back(tileset);

    if (tileset->getHeight() > mMaxTileHeight)
        mMaxTileHeight = tileset->getHeight();
}

bool spriteCompare(const Sprite *a, const Sprite *b)
{
    return a->getPixelY() < b->getPixelY();
}

void Map::update()
{
    //update animated tiles
    for (std::map<int, TileAnimation*>::iterator iAni = mTileAnimations.begin();
         iAni != mTileAnimations.end();
         iAni++)
    {
        iAni->second->update();
    }
}

void Map::draw(Graphics *graphics, int scrollX, int scrollY)
{
    int endPixelY = graphics->getHeight() + scrollY + mTileHeight - 1;

    // TODO: Do this per-layer
    endPixelY += mMaxTileHeight - mTileHeight;

    int startX = scrollX / mTileWidth;
    int startY = scrollY / mTileHeight;
    int endX = (graphics->getWidth() + scrollX + mTileWidth - 1) / mTileWidth;
    int endY = endPixelY / mTileHeight;

    // Make sure sprites are sorted
    mSprites.sort(spriteCompare);

    // draw the game world
    Layers::const_iterator layeri = mLayers.begin();
    for (; layeri != mLayers.end(); ++layeri)
    {
        (*layeri)->draw(graphics,
                        startX, startY, endX, endY,
                        scrollX, scrollY,
                        mSprites);
    }

    drawOverlay(graphics, scrollX, scrollY,
            (int) config.getValue("OverlayDetail", 2));
}

void Map::drawOverlay(Graphics *graphics,
                      float scrollX, float scrollY, int detail)
{
    static int lastTick = tick_time;

    // Detail 0: no overlays
    if (detail <= 0) return;

    if (mLastScrollX == 0.0f && mLastScrollY == 0.0f)
    {
        // First call - initialisation
        mLastScrollX = scrollX;
        mLastScrollY = scrollY;
    }

    // Update Overlays
    int timePassed = get_elapsed_time(lastTick);
    float dx = scrollX - mLastScrollX;
    float dy = scrollY - mLastScrollY;

    std::list<AmbientOverlay*>::iterator i;
    for (i = mOverlays.begin(); i != mOverlays.end(); i++)
    {
        (*i)->update(timePassed, dx, dy);
    }
    mLastScrollX = scrollX;
    mLastScrollY = scrollY;
    lastTick = tick_time;

    // Draw overlays
    for (i = mOverlays.begin(); i != mOverlays.end(); i++)
    {
        (*i)->draw(graphics, graphics->getWidth(), graphics->getHeight());

        // Detail 1: only one overlay, higher: all overlays
        if (detail == 1)
            break;
    };
}

class ContainsGidFunctor
{
    public:
        bool operator() (const Tileset* set) const
        {
            return (set->getFirstGid() <= gid &&
                    gid - set->getFirstGid() < (int)set->size());
        }
        int gid;
} containsGid;

Tileset* Map::getTilesetWithGid(int gid) const
{
    containsGid.gid = gid;

    Tilesets::const_iterator i = find_if(mTilesets.begin(), mTilesets.end(),
              containsGid);

    return (i == mTilesets.end()) ? NULL : *i;
}

void Map::setWalk(int x, int y, bool walkable)
{
    mMetaTiles[x + y * mWidth].walkable = walkable;
}
 
bool Map::occupied(int x, int y) const
{
    Beings &beings = beingManager->getAll();
    for (BeingIterator i = beings.begin(); i != beings.end(); i++)
    {
        // job 45 is a portal, they don't collide
        if ((*i)->mX == x && (*i)->mY == y && (*i)->mJob != 45)
        {
            return true;
        }
    }

    return false;
}

bool Map::tileCollides(int x, int y) const
{
     return !(contains(x, y) && mMetaTiles[x + y * mWidth].walkable);
}

bool Map::contains(int x, int y) const
{
    return x >= 0 && y >= 0 && x < mWidth && y < mHeight;
}

MetaTile* Map::getMetaTile(int x, int y)
{
    return &mMetaTiles[x + y * mWidth];
}

SpriteIterator Map::addSprite(Sprite *sprite)
{
    mSprites.push_front(sprite);
    return mSprites.begin();
}

void Map::removeSprite(SpriteIterator iterator)
{
    mSprites.erase(iterator);
}

Path Map::findPath(int startX, int startY, int destX, int destY)
{
    // Path to be built up (empty by default)
    Path path;

    // Declare open list, a list with open tiles sorted on F cost
    std::priority_queue<Location> openList;

    // Reset starting tile's G cost to 0
    MetaTile *startTile = getMetaTile(startX, startY);
    startTile->Gcost = 0;

    // Add the start point to the open list
    openList.push(Location(startX, startY, startTile));

    bool foundPath = false;

    // Keep trying new open tiles until no more tiles to try or target found
    while (!openList.empty() && !foundPath)
    {
        // Take the location with the lowest F cost from the open list.
        Location curr = openList.top();
        openList.pop();

        // If the tile is already on the closed list, this means it has already
        // been processed with a shorter path to the start point (lower G cost)
        if (curr.tile->whichList == mOnClosedList)
        {
            continue;
        }

        // Put the current tile on the closed list
        curr.tile->whichList = mOnClosedList;

        // Check the adjacent tiles
        for (int dy = -1; dy <= 1; dy++)
        {
            for (int dx = -1; dx <= 1; dx++)
            {
                // Calculate location of tile to check
                const int x = curr.x + dx;
                const int y = curr.y + dy;

                // Skip if if we're checking the same tile we're leaving from,
                // or if the new location falls outside of the map boundaries
                if ((dx == 0 && dy == 0) || !contains(x, y))
                {
                    continue;
                }

                MetaTile *newTile = getMetaTile(x, y);

                // Skip if the tile is on the closed list or collides unless
                // its the destination tile
                if (newTile->whichList == mOnClosedList ||
                        (tileCollides(x, y) && !(x == destX && y == destY)))
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
                // 14 for moving diagonal (sqrt(200) = 14.1421...)
                int Gcost = curr.tile->Gcost + ((dx == 0 || dy == 0) ? 10 : 14);

                // Skip if Gcost becomes too much
                // Warning: probably not entirely accurate
                if (Gcost > 200)
                {
                    continue;
                }

                if (newTile->whichList != mOnOpenList)
                {
                    // Found a new tile (not on open nor on closed list)
                    // Update Hcost of the new tile using Manhatten distance
                    newTile->Hcost = 10 * (abs(x - destX) + abs(y - destY));

                    // Set the current tile as the parent of the new tile
                    newTile->parentX = curr.x;
                    newTile->parentY = curr.y;

                    // Update Gcost and Fcost of new tile
                    newTile->Gcost = Gcost;
                    newTile->Fcost = Gcost + newTile->Hcost;

                    if (x != destX || y != destY) {
                        // Add this tile to the open list
                        newTile->whichList = mOnOpenList;
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
                    newTile->Fcost = Gcost + newTile->Hcost;

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

    // Two new values to indicate whether a tile is on the open or closed list,
    // this way we don't have to clear all the values between each pathfinding.
    mOnClosedList += 2;
    mOnOpenList += 2;

    // If a path has been found, iterate backwards using the parent locations
    // to extract it.
    if (foundPath)
    {
        int pathX = destX;
        int pathY = destY;

        while (pathX != startX || pathY != startY)
        {
            // Add the new path node to the start of the path list
            path.push_front(Position(pathX, pathY));

            // Find out the next parent
            MetaTile *tile = getMetaTile(pathX, pathY);
            pathX = tile->parentX;
            pathY = tile->parentY;
        }
    }

    return path;
}

void Map::addParticleEffect(const std::string &effectFile, int x, int y)
{
    ParticleEffectData newEffect;
    newEffect.file = effectFile;
    newEffect.x = x;
    newEffect.y = y;
    particleEffects.push_back(newEffect);
}

void Map::initializeParticleEffects(Particle* particleEngine)
{
    if (config.getValue("particleeffects", 1))
    {
        for (std::list<ParticleEffectData>::iterator i = particleEffects.begin();
             i != particleEffects.end();
             i++
            )
        {
            particleEngine->addEffect(i->file, i->x, i->y);
        }
    }
}

TileAnimation* Map::getAnimationForGid(int gid)
{
    std::map<int, TileAnimation*>::iterator i = mTileAnimations.find(gid);
    if (i == mTileAnimations.end())
    {
        return NULL;
    } else {
        return i->second;
    }
}
