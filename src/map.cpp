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

#include "map.h"

#include <algorithm>
#include <queue>

#include "beingmanager.h"
#include "graphics.h"
#include "sprite.h"
#include "tileset.h"

#include "resources/image.h"

#include "utils/dtor.h"

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

Map::Map(int width, int height, int tileWidth, int tileHeight):
    mWidth(width), mHeight(height),
    mTileWidth(tileWidth), mTileHeight(tileHeight),
    mOnClosedList(1), mOnOpenList(2),
    mLastScrollX(0.0f), mLastScrollY(0.0f)
{
    mMetaTiles = new MetaTile[mWidth * mHeight];
    mTiles = new Image*[mWidth * mHeight * 3];
}

Map::~Map()
{
    // clean up map data
    delete[] mMetaTiles;
    delete[] mTiles;
    // clean up tilesets
    for_each(mTilesets.begin(), mTilesets.end(), make_dtor(mTilesets));
    mTilesets.clear();
    // clean up overlays
    std::list<AmbientOverlay>::iterator i;
    for (i = mOverlays.begin(); i != mOverlays.end(); i++)
    {
        (*i).image->decRef();
    }
}

void
Map::setSize(int width, int height)
{
    mWidth = width;
    mHeight = height;
    delete[] mMetaTiles;
    delete[] mTiles;
    mMetaTiles = new MetaTile[mWidth * mHeight];
    mTiles = new Image*[mWidth * mHeight * 3];
}

void
Map::addTileset(Tileset *tileset)
{
    mTilesets.push_back(tileset);
}

bool spriteCompare(const Sprite *a, const Sprite *b)
{
    return a->getPixelY() < b->getPixelY();
}

void
Map::draw(Graphics *graphics, int scrollX, int scrollY, int layer)
{
    int startX = scrollX / 32;
    int startY = scrollY / 32;
    int endX = (graphics->getWidth() + scrollX + 31) / 32;
    int endY = (graphics->getHeight() + scrollY + 31) / 32;

    // If drawing the fringe layer, make sure sprites are sorted
    SpriteIterator si;
    if (layer == 1)
    {
        mSprites.sort(spriteCompare);
        si = mSprites.begin();

        // Increase endY to account for high fringe tiles
        // TODO: Improve this hack so that it'll dynamically account for the
        //       highest tile.
        endY += 2;
    }

    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX >= mWidth) endX = mWidth - 1;
    if (endY >= mHeight) endY = mHeight - 1;

    for (int y = startY; y < endY; y++)
    {
        // If drawing the fringe layer, make sure all sprites above this row of
        // tiles have been drawn
        if (layer == 1)
        {
            while (si != mSprites.end() && (*si)->getPixelY() <= y * 32 - 32)
            {
                (*si)->draw(graphics, -scrollX, -scrollY);
                si++;
            }
        }

        for (int x = startX; x < endX; x++)
        {
            Image *img = getTile(x, y, layer);
            if (img) {
                graphics->drawImage(img,
                                    x * 32 - scrollX,
                                    y * 32 - scrollY + 32 - img->getHeight());
            }
        }
    }

    // Draw any remaining sprites
    if (layer == 1)
    {
        while (si != mSprites.end())
        {
            (*si)->draw(graphics, -scrollX, -scrollY);
            si++;
        }
    }
}

void
Map::drawOverlay(Graphics *graphics, float scrollX, float scrollY, int detail)
{
    static int lastTick = tick_time;

    // detail 0: no overlays
    if (detail <= 0) return;

    std::list<AmbientOverlay>::iterator i;

    // Avoid freaking out when tick_time overflows
    if (tick_time < lastTick)
    {
        lastTick = tick_time;
    }

    if (mLastScrollX == 0.0f && mLastScrollY == 0.0f)
    {
        // first call - initialisation
        mLastScrollX = scrollX;
        mLastScrollY = scrollY;
    }

    //update Overlays
    while (lastTick < tick_time)
    {
        for (i = mOverlays.begin(); i != mOverlays.end(); i++)
        {
            if ((*i).image != NULL)
            {
                //apply self scrolling
                (*i).scrollX -= (*i).scrollSpeedX;
                (*i).scrollY -= (*i).scrollSpeedY;

                //apply parallaxing
                (*i).scrollX += (scrollX - mLastScrollX) * (*i).parallax;
                (*i).scrollY += (scrollY - mLastScrollY) * (*i).parallax;

                //keep the image pattern on the screen
                while ((*i).scrollX > (*i).image->getWidth())
                {
                    (*i).scrollX -= (*i).image->getWidth();
                }
                while ((*i).scrollY > (*i).image->getHeight())
                {
                    (*i).scrollY -= (*i).image->getHeight();
                }
                while ((*i).scrollX < 0)
                {
                    (*i).scrollX += (*i).image->getWidth();
                }
                while ((*i).scrollY < 0)
                {
                    (*i).scrollY += (*i).image->getHeight();
                }
            }
        }
        mLastScrollX = scrollX;
        mLastScrollY = scrollY;
        lastTick++;

        // detail 1: only one overlay, higher: all overlays
        if (detail == 1) break;
    }

    //draw overlays
    for (i = mOverlays.begin(); i != mOverlays.end(); i++)
    {
        if ((*i).image != NULL)
        {
        graphics->drawImagePattern  (   (*i).image,
                                        0 - (int)(*i).scrollX,
                                        0 - (int)(*i).scrollY,
                                        graphics->getWidth() + (int)(*i).scrollX,
                                        graphics->getHeight() + (int)(*i).scrollY
                                    );
        };
        // detail 1: only one overlay, higher: all overlays
        if (detail == 1) break;
    };
}

void
Map::setOverlay(Image *image, float speedX, float speedY, float parallax)
{
    if (image != NULL)
    {
        AmbientOverlay newOverlay;

        newOverlay.image = image;
        newOverlay.parallax = parallax;
        newOverlay.scrollSpeedX = speedX;
        newOverlay.scrollSpeedY = speedY;
        newOverlay.scrollX = 0;
        newOverlay.scrollY = 0;

        mOverlays.push_back(newOverlay);
    }
}

void
Map::setTileWithGid(int x, int y, int layer, int gid)
{
    if (layer == 3)
    {
        Tileset *set = getTilesetWithGid(gid);
        setWalk(x, y, (!set || (gid - set->getFirstGid() == 0)));
    }
    else if (layer < 3)
    {
        setTile(x, y, layer, getTileWithGid(gid));
    }
}

class ContainsGidFunctor
{
    public:
        bool operator() (Tileset* set)
        {
            return (set->getFirstGid() <= gid &&
                    gid - set->getFirstGid() < (int)set->size());
        }
        int gid;
} containsGid;

Tileset*
Map::getTilesetWithGid(int gid)
{
    containsGid.gid = gid;

    TilesetIterator i = find_if(mTilesets.begin(), mTilesets.end(),
            containsGid);

    return (i == mTilesets.end()) ? NULL : *i;
}

Image*
Map::getTileWithGid(int gid)
{
    Tileset *set = getTilesetWithGid(gid);

    if (set) {
        return set->get(gid - set->getFirstGid());
    }

    return NULL;
}

void
Map::setWalk(int x, int y, bool walkable)
{
    mMetaTiles[x + y * mWidth].walkable = walkable;
}

bool
Map::getWalk(int x, int y)
{
    // Check for being walkable
    if (tileCollides(x, y)) {
        return false;
    }

    // Check for collision with a being
    Beings *beings = beingManager->getAll();
    for (BeingIterator i = beings->begin(); i != beings->end(); i++) {
        // job 45 is a portal, they don't collide
        if ((*i)->mX / 32 == x && (*i)->mY / 32 == y && (*i)->mJob != 45) {
            return false;
        }
    }

    return true;
}

bool
Map::tileCollides(int x, int y)
{
    // You can't walk outside of the map
    if (x < 0 || y < 0 || x >= mWidth || y >= mHeight) {
        return true;
    }

    // Check if the tile is walkable
    return !mMetaTiles[x + y * mWidth].walkable;
}

void
Map::setTile(int x, int y, int layer, Image *img)
{
    mTiles[x + y * mWidth + layer * (mWidth * mHeight)] = img;
}

Image*
Map::getTile(int x, int y, int layer)
{
    return mTiles[x + y * mWidth + layer * (mWidth * mHeight)];
}

MetaTile*
Map::getMetaTile(int x, int y)
{
    return &mMetaTiles[x + y * mWidth];
}

SpriteIterator
Map::addSprite(Sprite *sprite)
{
    mSprites.push_front(sprite);
    return mSprites.begin();
}

void
Map::removeSprite(SpriteIterator iterator)
{
    mSprites.erase(iterator);
}

Path
Map::findPath(int startX, int startY, int destX, int destY)
{
    // Path to be built up (empty by default)
    Path path;

    // Declare open list, a list with open tiles sorted on F cost
    std::priority_queue<Location> openList;

    // Return empty path when destination not walkable
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
                int x = curr.x + dx;
                int y = curr.y + dy;

                // Skip if if we're checking the same tile we're leaving from,
                // or if the new location falls outside of the map boundaries
                if ((dx == 0 && dy == 0) ||
                    (x < 0 || y < 0 || x >= mWidth || y >= mHeight))
                {
                    continue;
                }

                MetaTile *newTile = getMetaTile(x, y);

                // Skip if the tile is on the closed list or is not walkable
                if (newTile->whichList == mOnClosedList || !getWalk(x, y))
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
                    newTile->Fcost = newTile->Gcost + newTile->Hcost;

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
            path.push_front(PATH_NODE(pathX, pathY));

            // Find out the next parent
            MetaTile *tile = getMetaTile(pathX, pathY);
            pathX = tile->parentX;
            pathY = tile->parentY;
        }
    }

    return path;
}
