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

#include "map.h"

#include "actorspritemanager.h"
#include "configuration.h"
#include "graphics.h"
#include "log.h"
#include "particle.h"
#include "simpleanimation.h"
#include "tileset.h"

#include "resources/ambientlayer.h"
#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "utils/time.h"

#include "net/net.h"

#include "utils/dtor.h"
#include "utils/stringutils.h"

#include <queue>
#include <climits>

/**
 * A location on a tile map. Used for pathfinding, open list.
 */
struct Location
{
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

TileAnimation::TileAnimation(Animation animation)
    : mAnimation(std::move(animation))
{
}

void TileAnimation::update(int dt)
{
    mAnimation.update(dt);

    // exchange images
    Image *img = mAnimation.getCurrentImage();
    if (img != mLastImage)
    {
        for (auto &[layer, index] : mAffected)
        {
            layer->setTile(index, img);
        }
        mLastImage = img;
    }
}

MapLayer::MapLayer(int x, int y, int width, int height, bool isFringeLayer,
                   Map *map):
    mX(x), mY(y),
    mWidth(width), mHeight(height),
    mIsFringeLayer(isFringeLayer),
    mMap(map)
{
    const int size = mWidth * mHeight;
    mTiles = new Image*[size];
    std::fill_n(mTiles, size, (Image*) nullptr);
}

MapLayer::~MapLayer()
{
    delete[] mTiles;
}

void MapLayer::setTile(int x, int y, Image *img)
{
    setTile(x + y * mWidth, img);
}

void MapLayer::draw(Graphics *graphics,
                    int startX, int startY,
                    int endX, int endY,
                    int scrollX, int scrollY,
                    const Actors &actors, int debugFlags) const
{
    startX -= mX;
    startY -= mY;
    endX -= mX;
    endY -= mY;

    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX > mWidth) endX = mWidth;
    if (endY > mHeight) endY = mHeight;

    auto ai = actors.begin();

    int dx = (mX * mMap->getTileWidth()) - scrollX;
    int dy = (mY * mMap->getTileHeight()) - scrollY + mMap->getTileHeight();

    for (int y = startY; y < endY; y++)
    {
        int pixelY = y * mMap->getTileHeight();

        // If drawing the fringe layer, make sure all actors above this row of
        // tiles have been drawn
        if (mIsFringeLayer)
        {
            while (ai != actors.end() && (*ai)->getDrawOrder()
                   <= y * mMap->getTileHeight())
            {
                (*ai)->draw(graphics, -scrollX, -scrollY);
                ++ai;
            }
        }

        if (!(debugFlags & Map::DEBUG_SPECIAL3))
        {
            const int py0 = pixelY + dy;

            for (int x = startX; x < endX; x++)
            {
                Image *img = getTile(x, y);
                if (img)
                {
                    const int px = (x * mMap->getTileWidth()) + dx;
                    const int py = py0 - img->getHeight();
                    if (!(debugFlags & (Map::DEBUG_SPECIAL1 | Map::DEBUG_SPECIAL2))
                        || img->getHeight() <= mMap->getTileHeight())
                    {
                        int width = 0;
                        int c = getTileDrawWidth(x, y, endX, width);
                        if (!c)
                        {
                            graphics->drawImage(img, px, py);
                        }
                        else
                        {
                            graphics->drawImagePattern(img, px, py,
                                width, img->getHeight());
                        }
                        x += c;
                    }
                }
            }
        }
    }

    // Draw any remaining actors
    if (mIsFringeLayer)
    {
        for (; ai != actors.end(); ++ai)
        {
            (*ai)->draw(graphics, -scrollX, -scrollY);
        }
    }
}

int MapLayer::getTileDrawWidth(int x1, int y1, int endX, int &width) const
{
    Image *img1 = getTile(x1, y1);
    int c = 0;
    width = img1->getWidth();

    // Images that don't match the tile width can't be drawn as a pattern
    if (width != mMap->getTileWidth())
        return c;

    for (int x = x1 + 1; x < endX; x++)
    {
        Image *img = getTile(x, y1);
        if (img != img1)
            break;
        c++;
        width += img->getWidth();
    }
    return c;
}

Map::Map(int width, int height, int tileWidth, int tileHeight):
    mWidth(width), mHeight(height),
    mTileWidth(tileWidth), mTileHeight(tileHeight),
    mMaxTileHeight(tileHeight),
    mMaxTileWidth(tileWidth),
    mDebugFlags(DEBUG_NONE),
    mOnClosedList(1), mOnOpenList(2),
    mLastScrollX(0.0f), mLastScrollY(0.0f)
{
    const int size = mWidth * mHeight;

    mMetaTiles = new MetaTile[size];
    for (auto &occupation : mOccupation)
    {
        occupation = new unsigned[size];
        memset(occupation, 0, size * sizeof(unsigned));
    }
}

Map::~Map()
{
    // delete metadata, layers, tilesets and overlays
    delete[] mMetaTiles;
    for (auto &occupation : mOccupation)
    {
        delete[] occupation;
    }
    delete_all(mLayers);
    delete_all(mTilesets);
}

void Map::initializeAmbientLayers()
{
    ResourceManager *resman = ResourceManager::getInstance();

    auto addAmbientLayer = [=](const std::string &name, std::vector<AmbientLayer> &list)
    {
        if (auto img = resman->getImageRef(getProperty(name + "image")))
        {
            auto &ambientLayer = list.emplace_back(img);
            ambientLayer.mParallax = getFloatProperty(name + "parallax");
            ambientLayer.mSpeedX = getFloatProperty(name + "scrollX") / MILLISECONDS_IN_A_TICK;
            ambientLayer.mSpeedY = getFloatProperty(name + "scrollY") / MILLISECONDS_IN_A_TICK;
            ambientLayer.mMask = getIntProperty(name + "mask", 1);
            ambientLayer.mKeepRatio = getBoolProperty(name + "keepratio");
        }
    };

    // search for "foreground*" or "overlay*" (old term) in map properties
    for (int i = 0; /* terminated by a break */; i++)
    {
        if (hasProperty("foreground" + toString(i) + "image"))
        {
            addAmbientLayer("foreground" + toString(i), mForegrounds);
        }
        else if (hasProperty("overlay" + toString(i) + "image"))
        {
            addAmbientLayer("overlay" + toString(i), mForegrounds);
        }
        else
        {
            break; // the FOR loop
        }
    }

    // search for "background*" in map properties
    for (int i = 0;
         hasProperty("background" + toString(i) + "image");
         i++)
    {
        addAmbientLayer("background" + toString(i), mBackgrounds);
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

void Map::update(int dt)
{
    // Update animated tiles
    for (auto &[_, tileAnimation] : mTileAnimations)
    {
        tileAnimation.update(dt);
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

    // Make sure actors are sorted ascending by Y-coordinate
    // so that they overlap correctly
    mActors.sort([] (const Actor *a, const Actor *b) {
        return a->getDrawOrder() < b->getDrawOrder();
    });

    // update scrolling of all ambient layers
    updateAmbientLayers(scrollX, scrollY);

    // Draw backgrounds
    drawAmbientLayers(graphics, BACKGROUND_LAYERS, scrollX, scrollY,
                      config.getIntValue("OverlayDetail"));

    // draw the game world
    for (auto &layer : mLayers)
    {
        if ((layer->getMask() & mMask) == 0)
            continue;

        if (!layer->isFringeLayer() && (mDebugFlags & DEBUG_SPECIAL3))
            continue;

        layer->draw(graphics,
                    startX, startY, endX, endY,
                    scrollX, scrollY,
                    mActors, mDebugFlags);

        if (layer->isFringeLayer() && (mDebugFlags & (DEBUG_SPECIAL2 |
                                                      DEBUG_SPECIAL3)))
            break;
    }

    // If the transparency hasn't been disabled,
    if (Image::useOpenGL() || !Image::SDLisTransparencyDisabled())
    {
        // We draw beings with a lower opacity to make them visible
        // even when covered by a wall or some other elements...
        for (auto actor : mActors)
        {
            // For now, just draw actors with only one layer.
            if (actor->drawnWhenBehind())
            {
                actor->setAlpha(0.3f);
                actor->draw(graphics, -scrollX, -scrollY);
                actor->setAlpha(1.0f);
            }
        }
    }

    drawAmbientLayers(graphics, FOREGROUND_LAYERS, scrollX, scrollY,
                      config.getIntValue("OverlayDetail"));
}

void Map::drawCollision(Graphics *graphics, int scrollX, int scrollY,
                        int debugFlags) const
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
            if (debugFlags & DEBUG_GRID)
            {
                graphics->drawRectangle(gcn::Rectangle(
                    x * mTileWidth - scrollX,
                    y * mTileHeight - scrollY,
                    mTileWidth + 1, mTileHeight + 1));
            }

            if (!(debugFlags & DEBUG_COLLISION_TILES))
                continue;

            if (!getWalk(x, y, BLOCKMASK_WALL))
            {
                graphics->setColor(gcn::Color(0, 0, 200, 64));
                graphics->fillRectangle(gcn::Rectangle(
                    x * mTileWidth - scrollX,
                    y * mTileHeight - scrollY,
                    mTileWidth, mTileHeight));
            }

            if (!getWalk(x, y, BLOCKMASK_MONSTER))
            {
                graphics->setColor(gcn::Color(200, 0, 0, 64));
                graphics->fillRectangle(gcn::Rectangle(
                    x * mTileWidth - scrollX,
                    y * mTileHeight - scrollY,
                    mTileWidth, mTileHeight));
            }

            if (!getWalk(x, y, BLOCKMASK_CHARACTER))
            {
                graphics->setColor(gcn::Color(0, 200, 0, 64));
                graphics->fillRectangle(gcn::Rectangle(
                    x * mTileWidth - scrollX,
                    y * mTileHeight - scrollY,
                    mTileWidth, mTileHeight));
            }
        }
    }
}

void Map::updateAmbientLayers(float scrollX, float scrollY)
{
    if (mLastScrollX == 0.0f && mLastScrollY == 0.0f)
    {
        // First call - initialisation
        mLastScrollX = scrollX;
        mLastScrollY = scrollY;
    }

    // Update Overlays
    float dx = scrollX - mLastScrollX;
    float dy = scrollY - mLastScrollY;

    for (auto &background : mBackgrounds)
    {
        if ((background.mMask & mMask) == 0)
            continue;

        background.update(Time::deltaTimeMs(), dx, dy);
    }
    for (auto &foreground : mForegrounds)
    {
        if ((foreground.mMask & mMask) == 0)
            continue;

        foreground.update(Time::deltaTimeMs(), dx, dy);
    }
    mLastScrollX = scrollX;
    mLastScrollY = scrollY;
}

void Map::drawAmbientLayers(Graphics *graphics, LayerType type,
                            float scrollX, float scrollY, int detail)
{
    // Detail 0 = no ambient effects except background image
    if (detail <= 0 && type != BACKGROUND_LAYERS) return;

    // find out which layer list to draw
    std::vector<AmbientLayer> *layers;
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
            return;
    }

    // Draw overlays
    for (auto &layer : *layers)
    {
        if ((layer.mMask & mMask) == 0)
            continue;

        layer.draw(graphics);

        // Detail 1: only one overlay, higher: all overlays
        if (detail == 1)
            break;
    }
}

Tileset *Map::getTilesetWithGid(unsigned gid) const
{
    Tileset *s = nullptr;
    for (auto it = mTilesets.begin(),
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

    if (mOccupation[type][tileNum] < UINT_MAX &&
        (++mOccupation[type][tileNum]) > 0)
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
    for (auto actor : actorSpriteManager->getAll())
    {
        if (actor->getTileX() == x && actor->getTileY() == y &&
            actor->getType() != ActorSprite::FLOOR_ITEM)
            return true;
    }

    return false;
}

Vector Map::getTileCenter(int x, int y) const
{
    Vector tileCenterPos;

    tileCenterPos.x = x * mTileWidth + mTileWidth / 2;
    tileCenterPos.y = y * mTileHeight + mTileHeight / 2;
    return tileCenterPos;
}

bool Map::contains(int x, int y) const
{
    return x >= 0 && y >= 0 && x < mWidth && y < mHeight;
}

MetaTile *Map::getMetaTile(int x, int y) const
{
    return &mMetaTiles[x + y * mWidth];
}

Actors::iterator Map::addActor(Actor *actor)
{
    mActors.push_front(actor);
    return mActors.begin();
}

void Map::removeActor(Actors::iterator iterator)
{
    mActors.erase(iterator);
}

std::string Map::getMusicFile() const
{
    return getProperty("music");
}

std::string Map::getName() const
{
    if (hasProperty("name"))
        return getProperty("name");

    return getProperty("mapname");
}

std::string Map::getFilename() const
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
    const int tx = position.x / mTileWidth;
    const int ty = position.y / mTileHeight;

    // Pre-computing character's position offsets.
    int fx = position.x % mTileWidth;
    int fy = position.y % mTileHeight;

    // Compute the being radius:
    // FIXME: Hande beings with more than 1/2 tile radius by not letting them
    // go or spawn in too narrow places. The server will have to be aware
    // of being's radius value (in tiles) to handle this gracefully.
    if (radius > mTileWidth / 2) radius = mTileWidth / 2;
    // set a default value if no value returned.
    if (radius < 1) radius = mTileWidth / 3;

    // We check diagonal first as they are more restrictive.
    // Top-left border check
    if (!getWalk(tx - 1, ty - 1, walkMask)
        && fy < radius && fx < radius)
    {
        fx = fy = radius;
    }
    // Top-right border check
    if (!getWalk(tx + 1, ty - 1, walkMask)
        && (fy < radius) && fx > (mTileWidth - radius))
    {
        fx = mTileWidth - radius;
        fy = radius;
    }
    // Bottom-left border check
    if (!getWalk(tx - 1, ty + 1, walkMask)
        && fy > (mTileHeight - radius) && fx < radius)
    {
        fx = radius;
        fy = mTileHeight - radius;
    }
    // Bottom-right border check
    if (!getWalk(tx + 1, ty + 1, walkMask)
        && fy > (mTileHeight - radius) && fx > (mTileWidth - radius))
    {
        fx = mTileWidth - radius;
        fy = mTileHeight - radius;
    }

    // Fix coordinates so that the player does not seem to dig into walls.
    if (fx > (mTileWidth - radius) && !getWalk(tx + 1, ty, walkMask))
        fx = mTileWidth - radius;
    else if (fx < radius && !getWalk(tx - 1, ty, walkMask))
        fx = radius;
    else if (fy > (mTileHeight - radius) && !getWalk(tx, ty + 1, walkMask))
        fy = mTileHeight - radius;
    else if (fy < radius && !getWalk(tx, ty - 1, walkMask))
        fy = radius;

    return Position(tx * mTileWidth + fx, ty * mTileHeight + fy);
}

Path Map::findTilePath(int startPixelX, int startPixelY, int endPixelX,
                         int endPixelY, unsigned char walkMask, int maxCost)
{
    Path myPath = findPath(startPixelX / mTileWidth, startPixelY / mTileHeight,
                           endPixelX / mTileWidth, endPixelY / mTileHeight,
                           walkMask, maxCost);

    // Don't compute empty coordinates.
    if (myPath.empty())
        return myPath;

    // Convert the map path to pixels from the tile position
    auto it = myPath.begin();
    while (it != myPath.end())
    {
        // The new pixel position will be the tile center.
        *it = Position(it->x * mTileWidth + mTileWidth / 2,
                       it->y * mTileHeight + mTileHeight / 2);
        ++it;
    }

    return myPath;
}

Path Map::findPixelPath(int startPixelX, int startPixelY, int endPixelX,
                         int endPixelY,
                         int radius, unsigned char walkMask, int maxCost)
{
    Path myPath = findPath(startPixelX / mTileWidth, startPixelY / mTileHeight,
                           endPixelX / mTileWidth, endPixelY / mTileHeight,
                           walkMask, maxCost);

    // Don't compute empty coordinates.
    if (myPath.empty())
        return myPath;

    // Find the starting offset
    float startOffsetX = (startPixelX % mTileWidth);
    float startOffsetY = (startPixelY % mTileHeight);

    // Find the ending offset
    float endOffsetX = (endPixelX % mTileWidth);
    float endOffsetY = (endPixelY % mTileHeight);

    // Find the distance, and divide it by the number of steps
    int changeX = (int)((endOffsetX - startOffsetX) / myPath.size());
    int changeY = (int)((endOffsetY - startOffsetY) / myPath.size());

    // Convert the map path to pixels over tiles
    // And add interpolation between the starting and ending offsets
    auto it = myPath.begin();
    int i = 0;
    while (it != myPath.end())
    {
        // A position that is valid on the start and end tile is not
        // necessarily valid on all the tiles in between, so check the offsets.
        *it = checkNodeOffsets(radius, walkMask,
                               it->x * mTileWidth + startOffsetX + changeX * i,
                               it->y * mTileHeight + startOffsetY + changeY * i);
        ++i;
        ++it;
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
    // The basic walking cost of a tile.
    const int basicCost = 100;

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
    openList.emplace(startX, startY, startTile);

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
            continue;

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
                    continue;

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

                    if ((t1->blockmask | t2->blockmask) & walkmask)
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
                // N.B.: Specific to TmwAthena for now.
                if (Net::getNetworkType() == ServerType::TMWATHENA &&
                    occupied(x, y))
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
                    int dx = std::abs(x - destX);
                    int dy = std::abs(y - destY);
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
                        openList.emplace(x, y, newTile);
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
                    openList.emplace(x, y, newTile);
                }
            }
        }
    }

    // Two new values to indicate whether a tile is on the open or closed list,
    // this way we don't have to clear all the values between each pathfinding.
    if (mOnOpenList > UINT_MAX - 2)
    {
        // We reset the list memebers value.
        mOnClosedList = 1;
        mOnOpenList = 2;

        // Clean up the metaTiles
        const int size = mWidth * mHeight;
        for (int i = 0; i < size; ++i)
            mMetaTiles[i].whichList = 0;
    }
    else
    {
        mOnClosedList += 2;
        mOnOpenList += 2;
    }

    // If a path has been found, iterate backwards using the parent locations
    // to extract it.
    if (foundPath)
    {
        int pathX = destX;
        int pathY = destY;

        while (pathX != startX || pathY != startY)
        {
            // Add the new path node to the start of the path list
            path.emplace_front(pathX, pathY);

            // Find out the next parent
            MetaTile *tile = getMetaTile(pathX, pathY);
            pathX = tile->parentX;
            pathY = tile->parentY;
        }
    }

    return path;
}

void Map::addParticleEffect(const std::string &effectFile, int x, int y, int w,
                            int h)
{
    ParticleEffectData &newEffect = particleEffects.emplace_back();
    newEffect.file = effectFile;
    newEffect.x = x;
    newEffect.y = y;
    newEffect.w = w;
    newEffect.h = h;
}

void Map::initializeParticleEffects(Particle *particleEngine)
{
    if (config.getBoolValue("particleeffects"))
    {
        for (auto &particleEffect : particleEffects)
        {
            Particle *p = particleEngine->addEffect(particleEffect.file,
                                                    particleEffect.x,
                                                    particleEffect.y);

            if (p && particleEffect.w > 0 && particleEffect.h > 0)
            {
                p->adjustEmitterSize(particleEffect.w, particleEffect.h);
            }
        }
    }
}

void Map::addAnimation(int gid, TileAnimation animation)
{
    auto const [_, inserted] = mTileAnimations.try_emplace(gid, std::move(animation));
    if (!inserted)
    {
        logger->error(strprintf("Duplicate tile animation for gid %d", gid));
    }
}

TileAnimation *Map::getAnimationForGid(int gid)
{
    auto i = mTileAnimations.find(gid);
    return i == mTileAnimations.end() ? nullptr : &i->second;
}

void Map::setMask(int mask)
{
    mMask = mask;
}
