/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "map.h"

#include "beingmanager.h"
#include "client.h"
#include "configuration.h"
#include "graphics.h"
#include "particle.h"
#include "simpleanimation.h"
#include "sprite.h"
#include "tileset.h"

#include "resources/ambientlayer.h"
#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "utils/dtor.h"
#include "utils/stringutils.h"

#include <queue>

/**
 * A location on a tile map. Used for pathfinding, open list.
 */
struct Location
{
    /**
     * Constructor.
     */
    Location(int px, int py, MetaTile *ptile):
        x(px), y(py), tile(ptile)
    {}

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

void TileAnimation::update(int ticks)
{
    if (!mAnimation)
        return;

    // update animation
    mAnimation->update(ticks);

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
                    const MapSprites &sprites, int debugFlags) const
{
    startX -= mX;
    startY -= mY;
    endX -= mX;
    endY -= mY;

    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX > mWidth) endX = mWidth;
    if (endY > mHeight) endY = mHeight;

    MapSprites::const_iterator si = sprites.begin();

    for (int y = startY; y < endY; y++)
    {
        // If drawing the fringe layer, make sure all sprites above this row of
        // tiles have been drawn
        if (mIsFringeLayer)
        {
            while (si != sprites.end() && (*si)->getPixelY() <= y * 32)
            {
                (*si)->setAlpha(1.0f);
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
                if (debugFlags != Map::MAP_SPECIAL || img->getHeight() <= 32)
                    graphics->drawImage(img, px, py);
            }
        }
    }

    // Draw any remaining sprites
    if (mIsFringeLayer)
    {
        while (si != sprites.end())
        {
            (*si)->setAlpha(1.0f);
            (*si)->draw(graphics, -scrollX, -scrollY);
            si++;
        }
    }
}

Map::Map(int width, int height, int tileWidth, int tileHeight):
    mWidth(width), mHeight(height),
    mTileWidth(tileWidth), mTileHeight(tileHeight),
    mMaxTileHeight(tileHeight),
    mMaxTileWidth(tileWidth),
    mDebugFlags(MAP_NORMAL),
    mOnClosedList(1), mOnOpenList(2),
    mLastScrollX(0.0f), mLastScrollY(0.0f)
{
    const int size = mWidth * mHeight;

    mMetaTiles = new MetaTile[size];
    for (int i = 0; i < NB_BLOCKTYPES; i++)
    {
        mOccupation[i] = new int[size];
        memset(mOccupation[i], 0, size * sizeof(int));
    }
}

Map::~Map()
{
    // delete metadata, layers, tilesets and overlays
    delete[] mMetaTiles;
    for (int i = 0; i < NB_BLOCKTYPES; i++)
    {
        delete[] mOccupation[i];
    }
    delete_all(mLayers);
    delete_all(mTilesets);
    delete_all(mForegrounds);
    delete_all(mBackgrounds);
    delete_all(mTileAnimations);
}

void Map::initializeAmbientLayers()
{
    ResourceManager *resman = ResourceManager::getInstance();

    // search for "foreground*" or "overlay*" (old term) in map properties
    for (int i = 0; /* terminated by a break */; i++)
    {
        std::string name;
        if (hasProperty("foreground" + toString(i) + "image"))
        {
            name = "foreground" + toString(i);
        }
        else if (hasProperty("overlay" + toString(i) + "image"))
        {
            name = "overlay" + toString(i);
        }
        else
        {
            break; // the FOR loop
        }

        Image *img = resman->getImage(getProperty(name + "image"));
        const float speedX = getFloatProperty(name + "scrollX");
        const float speedY = getFloatProperty(name + "scrollY");
        const float parallax = getFloatProperty(name + "parallax");
        const bool keepRatio = getBoolProperty(name + "keepratio");

        if (img)
        {
            mForegrounds.push_back(
                    new AmbientLayer(img, parallax, speedX, speedY, keepRatio));

            // The AmbientLayer takes control over the image.
            img->decRef();
        }
    }


    // search for "background*" in map properties
    for (int i = 0;
         hasProperty("background" + toString(i) + "image");
         i++)
    {
        const std::string name = "background" + toString(i);

        Image *img = resman->getImage(getProperty(name + "image"));
        const float speedX = getFloatProperty(name + "scrollX");
        const float speedY = getFloatProperty(name + "scrollY");
        const float parallax = getFloatProperty(name + "parallax");
        const bool keepRatio = getBoolProperty(name + "keepratio");

        if (img)
        {
            mBackgrounds.push_back(
                    new AmbientLayer(img, parallax, speedX, speedY, keepRatio));

            // The AmbientLayer takes control over the image.
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
    if (tileset->getWidth() > mMaxTileWidth)
        mMaxTileWidth = tileset->getWidth();
}

bool spriteCompare(const Sprite *a, const Sprite *b)
{
    return a->getPixelY() < b->getPixelY();
}

void Map::update(int ticks)
{
    // Update animated tiles
    for (std::map<int, TileAnimation*>::iterator iAni = mTileAnimations.begin();
         iAni != mTileAnimations.end();
         iAni++)
    {
        iAni->second->update(ticks);
    }
}

void Map::draw(Graphics *graphics, int scrollX, int scrollY)
{
    // Calculate range of tiles which are on-screen
    int endPixelY = graphics->getHeight() + scrollY + mTileHeight - 1;
    endPixelY += mMaxTileHeight - mTileHeight;
    int startX = (scrollX - mMaxTileWidth + mTileWidth) / mTileWidth;
    int startY = scrollY / mTileHeight;
    int endX = (graphics->getWidth() + scrollX + mTileWidth - 1) / mTileWidth;
    int endY = endPixelY / mTileHeight;

    // Make sure sprites are sorted ascending by Y-coordinate
    // so that they overlap correctly
    mSprites.sort(spriteCompare);

    // update scrolling of all ambient layers
    updateAmbientLayers(scrollX, scrollY);

    // Draw backgrounds
    drawAmbientLayers(graphics, BACKGROUND_LAYERS, scrollX, scrollY,
            (int) config.getValue("OverlayDetail", 2));

    // draw the game world
    Layers::const_iterator layeri = mLayers.begin();
    for (; layeri != mLayers.end(); ++layeri)
    {
        (*layeri)->draw(graphics,
                        startX, startY, endX, endY,
                        scrollX, scrollY,
                        mSprites, mDebugFlags);
    }

    // If the transparency hasn't been disabled,
    if (Image::useOpenGL() || !Image::SDLisTransparencyDisabled())
    {
        // We draw beings with a lower opacity to make them visible
        // even when covered by a wall or some other elements...
        MapSprites::const_iterator si = mSprites.begin();
        while (si != mSprites.end())
        {
            if (Sprite *sprite = *si)
            {
                // For now, just draw sprites with only one layer.
                if (sprite->getNumberOfLayers() == 1)
                {
                    sprite->setAlpha(0.3f);
                    sprite->draw(graphics, -scrollX, -scrollY);
                }
            }
            si++;
        }
    }

    drawAmbientLayers(graphics, FOREGROUND_LAYERS, scrollX, scrollY,
            (int) config.getValue("OverlayDetail", 2));
}

void Map::drawCollision(Graphics *graphics, int scrollX, int scrollY,
                        int debugFlags)
{
    int endPixelY = graphics->getHeight() + scrollY + mTileHeight - 1;
    int startX = scrollX / mTileWidth;
    int startY = scrollY / mTileHeight;
    int endX = (graphics->getWidth() + scrollX + mTileWidth - 1) / mTileWidth;
    int endY = endPixelY / mTileHeight;

    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX > mWidth) endX = mWidth;
    if (endY > mHeight) endY = mHeight;

    for (int y = startY; y < endY; y++)
    {
        for (int x = startX; x < endX; x++)
        {

            graphics->setColor(gcn::Color(0, 0, 0, 64));
            if (debugFlags < MAP_SPECIAL)
            {
                graphics->drawRectangle(gcn::Rectangle(
                    x * mTileWidth - scrollX,
                    y * mTileHeight - scrollY,
                    33, 33));
            }

            if (!getWalk(x, y, BLOCKMASK_WALL))
            {
                graphics->setColor(gcn::Color(0, 0, 200, 64));
                graphics->fillRectangle(gcn::Rectangle(
                    x * mTileWidth - scrollX,
                    y * mTileHeight - scrollY,
                    32, 32));
            }

            if (!getWalk(x, y, BLOCKMASK_MONSTER))
            {
                graphics->setColor(gcn::Color(200, 0, 0, 64));
                graphics->fillRectangle(gcn::Rectangle(
                    x * mTileWidth - scrollX,
                    y * mTileHeight - scrollY,
                    32, 32));
            }

            if (!getWalk(x, y, BLOCKMASK_CHARACTER))
            {
                graphics->setColor(gcn::Color(0, 200, 0, 64));
                graphics->fillRectangle(gcn::Rectangle(
                    x * mTileWidth - scrollX,
                    y * mTileHeight - scrollY,
                    32, 32));
            }
        }
    }
}

void Map::updateAmbientLayers(float scrollX, float scrollY)
{
    static int lastTick = tick_time; // static = only initialized at first call

    if (mLastScrollX == 0.0f && mLastScrollY == 0.0f)
    {
        // First call - initialisation
        mLastScrollX = scrollX;
        mLastScrollY = scrollY;
    }

    // Update Overlays
    float dx = scrollX - mLastScrollX;
    float dy = scrollY - mLastScrollY;
    int timePassed = get_elapsed_time(lastTick);

    std::list<AmbientLayer*>::iterator i;
    for (i = mBackgrounds.begin(); i != mBackgrounds.end(); i++)
    {
        (*i)->update(timePassed, dx, dy);
    }
    for (i = mForegrounds.begin(); i != mForegrounds.end(); i++)
    {
        (*i)->update(timePassed, dx, dy);
    }
    mLastScrollX = scrollX;
    mLastScrollY = scrollY;
    lastTick = tick_time;
}

void Map::drawAmbientLayers(Graphics *graphics, LayerType type,
                      float scrollX, float scrollY, int detail)
{
    // Detail 0 = no ambient effects except background image
    if (detail <= 0 && type != BACKGROUND_LAYERS) return;

    // find out which layer list to draw
    std::list<AmbientLayer*> *layers;
    switch (type)
    {
        case FOREGROUND_LAYERS:
            layers = &mForegrounds;
            break;
        case BACKGROUND_LAYERS:
            layers = &mBackgrounds;
            break;
        default:
            // New type of ambient layers added here without adding it
            // to Map::drawAmbientLayers.
            assert(false);
            break;
    }

    // Draw overlays
    for (std::list<AmbientLayer*>::iterator i = layers->begin();
         i != layers->end(); i++)
    {
        (*i)->draw(graphics, graphics->getWidth(), graphics->getHeight());

        // Detail 1: only one overlay, higher: all overlays
        if (detail == 1)
            break;
    }
}

Tileset *Map::getTilesetWithGid(int gid) const
{
    Tileset *s = NULL;
    for (Tilesets::const_iterator it = mTilesets.begin(),
         it_end = mTilesets.end(); it < it_end && (*it)->getFirstGid() <= gid;
         it++)
        s = *it;

    return s;
}

void Map::blockTile(int x, int y, BlockType type)
{
    if (type == BLOCKTYPE_NONE || !contains(x, y))
        return;

    const int tileNum = x + y * mWidth;

    if ((++mOccupation[type][tileNum]) > 0)
    {
        switch (type)
        {
            case BLOCKTYPE_WALL:
                mMetaTiles[tileNum].blockmask |= BLOCKMASK_WALL;
                break;
            case BLOCKTYPE_CHARACTER:
                mMetaTiles[tileNum].blockmask |= BLOCKMASK_CHARACTER;
                break;
            case BLOCKTYPE_MONSTER:
                mMetaTiles[tileNum].blockmask |= BLOCKMASK_MONSTER;
                break;
            default:
                // Do nothing.
                break;
        }
    }
}

bool Map::getWalk(int x, int y, unsigned char walkmask) const
{
    // You can't walk outside of the map
    if (!contains(x, y))
        return false;

    // Check if the tile is walkable
    return !(mMetaTiles[x + y * mWidth].blockmask & walkmask);
}

bool Map::occupied(int x, int y) const
{
    const Beings &beings = beingManager->getAll();
    for (Beings::const_iterator i = beings.begin(); i != beings.end(); i++)
    {
        const Being *being = *i;

        if (being->getTileX() == x && being->getTileY() == y)
            return true;
    }

    return false;
}

bool Map::contains(int x, int y) const
{
    return x >= 0 && y >= 0 && x < mWidth && y < mHeight;
}

MetaTile *Map::getMetaTile(int x, int y) const
{
    return &mMetaTiles[x + y * mWidth];
}

MapSprite Map::addSprite(Sprite *sprite)
{
    mSprites.push_front(sprite);
    return mSprites.begin();
}

void Map::removeSprite(MapSprite iterator)
{
    mSprites.erase(iterator);
}

const std::string Map::getMusicFile() const
{
    return getProperty("music");
}

const std::string Map::getName() const
{
    if (hasProperty("name"))
        return getProperty("name");

    return getProperty("mapname");
}

const std::string Map::getFilename() const
{
    std::string fileName = getProperty("_filename");
    int lastSlash = fileName.rfind("/") + 1;
    int lastDot = fileName.rfind(".");

    return fileName.substr(lastSlash, lastDot - lastSlash);
}

Position Map::checkNodeOffsets(int radius, unsigned char walkMask,
                               const Position &position) const
{
    // Pre-computing character's position in tiles
    const int tx = position.x / 32;
    const int ty = position.y / 32;

    // Pre-computing character's position offsets.
    int fx = position.x % 32;
    int fy = position.y % 32;

    // Compute the being radius:
    // FIXME: Hande beings with more than 1/2 tile radius by not letting them
    // go or spawn in too narrow places. The server will have to be aware
    // of being's radius value (in tiles) to handle this gracefully.
    if (radius > 32 / 2) radius = 32 / 2;
    // set a default value if no value returned.
    if (radius < 1) radius = 32 / 3;

    // We check diagonal first as they are more restrictive.
    // Top-left border check
    if (!getWalk(tx - 1, ty - 1, walkMask)
        && fy < radius && fx < radius)
    {
        fx = fy = radius;
    }
    // Top-right border check
    if (!getWalk(tx + 1, ty - 1, walkMask)
        && (fy < radius) && fx > (32 - radius))
    {
        fx = 32 -radius;
        fy = radius;
    }
    // Bottom-left border check
    if (!getWalk(tx - 1, ty + 1, walkMask)
        && fy > (32 - radius) && fx < radius)
    {
        fx = radius;
        fy = 32 - radius;
    }
    // Bottom-right border check
    if (!getWalk(tx + 1, ty + 1, walkMask)
        && fy > (32 - radius) && fx > (32 - radius))
    {
        fx = fy = 32 -radius;
    }

    // Fix coordinates so that the player does not seem to dig into walls.
    if (fx > (32 - radius) && !getWalk(tx + 1, ty, walkMask))
        fx = 32 - radius;
    else if (fx < radius && !getWalk(tx - 1, ty, walkMask))
        fx = radius;
    else if (fy > (32 - radius) && !getWalk(tx, ty + 1, walkMask))
        fy = 32 - radius;
    else if (fy < radius && !getWalk(tx, ty - 1, walkMask))
        fy = radius;

    return Position(tx * 32 + fx, ty * 32 + fy);
}

Path Map::findPixelPath(int startPixelX, int startPixelY, int endPixelX,
                         int endPixelY,
                         int radius, unsigned char walkMask, int maxCost)
{
    Path myPath = findPath(startPixelX / 32, startPixelY / 32,
                           endPixelX / 32, endPixelY / 32, walkMask, maxCost);

    // Don't compute empty coordinates.
    if (myPath.empty())
        return myPath;

    // Find the starting offset
    float startOffsetX = (startPixelX % 32);
    float startOffsetY = (startPixelY % 32);

    // Find the ending offset
    float endOffsetX = (endPixelX % 32);
    float endOffsetY = (endPixelY % 32);

    // Find the distance, and divide it by the number of steps
    int changeX = (int)((endOffsetX - startOffsetX) / myPath.size());
    int changeY = (int)((endOffsetY - startOffsetY) / myPath.size());

    // Convert the map path to pixels over tiles
    // And add interpolation between the starting and ending offsets
    Path::iterator it = myPath.begin();
    int i = 0;
    while (it != myPath.end())
    {
        // A position that is valid on the start and end tile is not
        // necessarily valid on all the tiles in between, so check the offsets.
        *it = checkNodeOffsets(radius, walkMask,
                               it->x * 32 + startOffsetX + changeX * i,
                               it->y * 32 + startOffsetY + changeY * i);
        i++;
        it++;
    }

    // Remove the last path node, as it's more clever to go to the destination.
    // It also permit to avoid zigzag at the end of the path,
    // especially with mouse.
    Position destination = checkNodeOffsets(radius, walkMask,
                                            endPixelX, endPixelY);
    myPath.pop_back();
    myPath.push_back(destination);

    return myPath;
}

Path Map::findPath(int startX, int startY, int destX, int destY,
                   unsigned char walkmask, int maxCost)
{
    static int const basicCost = 100;

    // Path to be built up (empty by default)
    Path path;

    // Declare open list, a list with open tiles sorted on F cost
    std::priority_queue<Location> openList;

    // Return when destination not walkable
    if (!getWalk(destX, destY, walkmask))
        return path;

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

                // Skip if the tile is on the closed list or is not walkable
                // unless its the destination tile
                if (newTile->whichList == mOnClosedList ||
                    ((newTile->blockmask & walkmask)
                     && !(x == destX && y == destY)))
                {
                    continue;
                }

                // When taking a diagonal step, verify that we can skip the
                // corner.
                if (dx != 0 && dy != 0)
                {
                    MetaTile *t1 = getMetaTile(curr.x, curr.y + dy);
                    MetaTile *t2 = getMetaTile(curr.x + dx, curr.y);

                    if ((t1->blockmask | t2->blockmask) & BLOCKMASK_WALL)
                        continue;
                }

                // Calculate G cost for this route, ~sqrt(2) for moving diagonal
                int Gcost = curr.tile->Gcost +
                    (dx == 0 || dy == 0 ? basicCost : basicCost * 362 / 256);

                /* Demote an arbitrary direction to speed pathfinding by
                   adding a defect (TODO: change depending on the desired
                   visual effect, e.g. a cross-product defect toward
                   destination).
                   Important: as long as the total defect along any path is
                   less than the basicCost, the pathfinder will still find one
                   of the shortest paths! */
                if (dx == 0 || dy == 0)
                {
                    // Demote horizontal and vertical directions, so that two
                    // consecutive directions cannot have the same Fcost.
                    ++Gcost;
                }

                // It costs extra to walk through a being (needs to be enough
                // to make it more attractive to walk around).
                if (occupied(x, y))
                {
                    Gcost += 3 * basicCost;
                }

                // Skip if Gcost becomes too much
                // Warning: probably not entirely accurate
                if (Gcost > maxCost * basicCost)
                {
                    continue;
                }

                if (newTile->whichList != mOnOpenList)
                {
                    // Found a new tile (not on open nor on closed list)

                    /* Update Hcost of the new tile. The pathfinder does not
                       work reliably if the heuristic cost is higher than the
                       real cost. In particular, using Manhattan distance is
                       forbidden here. */
                    int dx = std::abs(x - destX), dy = std::abs(y - destY);
                    newTile->Hcost = std::abs(dx - dy) * basicCost +
                        std::min(dx, dy) * (basicCost * 362 / 256);

                    // Set the current tile as the parent of the new tile
                    newTile->parentX = curr.x;
                    newTile->parentY = curr.y;

                    // Update Gcost and Fcost of new tile
                    newTile->Gcost = Gcost;
                    newTile->Fcost = Gcost + newTile->Hcost;

                    if (x != destX || y != destY)
                    {
                        // Add this tile to the open list
                        newTile->whichList = mOnOpenList;
                        openList.push(Location(x, y, newTile));
                    }
                    else
                    {
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

void Map::addParticleEffect(const std::string &effectFile, int x, int y, int w, int h)
{
    ParticleEffectData newEffect;
    newEffect.file = effectFile;
    newEffect.x = x;
    newEffect.y = y;
    newEffect.w = w;
    newEffect.h = h;
    particleEffects.push_back(newEffect);

}

void Map::initializeParticleEffects(Particle *particleEngine)
{
    Particle *p;

    if (config.getValue("particleeffects", 1))
    {
        for (std::list<ParticleEffectData>::iterator i = particleEffects.begin();
             i != particleEffects.end();
             i++
            )
        {
            p = particleEngine->addEffect(i->file, i->x, i->y);
            if (p && i->w > 0 && i->h > 0)
            {
                p->adjustEmitterSize(i->w, i->h);
            }
        }
    }
}

TileAnimation *Map::getAnimationForGid(int gid) const
{
    std::map<int, TileAnimation*>::const_iterator i = mTileAnimations.find(gid);
    return (i == mTileAnimations.end()) ? NULL : i->second;
}
