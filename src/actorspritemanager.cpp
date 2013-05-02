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

#include "actorspritemanager.h"

#include "game.h"
#include "localplayer.h"

#include "utils/dtor.h"

#include <cassert>

#define for_actors ActorSpritesConstIterator it, it_end; \
for (it = mActors.begin(), it_end = mActors.end() ; it != it_end; it++)

class FindBeingFunctor
{
    public:
        bool operator() (ActorSprite *actor)
        {
            if (actor->getType() == ActorSprite::FLOOR_ITEM)
                return false;
            Game *game = Game::instance();
            if (!game)
                return false;

            Being* b = static_cast<Being*>(actor);

            uint16_t other_y = y + ((b->getType() == ActorSprite::NPC) ? 1 : 0);
            const Vector &pos = b->getPosition();
            return ((int) pos.x / game->getCurrentTileWidth() == x &&
                    ((int) pos.y / game->getCurrentTileHeight() == y
                    || (int) pos.y / game->getCurrentTileHeight() == other_y) &&
                    b->isAlive() &&
                    (type == ActorSprite::UNKNOWN || b->getType() == type));
        }

        uint16_t x, y;
        ActorSprite::Type type;
} beingFinder;

class PlayerNamesLister : public AutoCompleteLister
{
    void getAutoCompleteList(std::vector<std::string>& names) const
    {
        names.clear();

        const ActorSprites &mActors = actorSpriteManager->getAll();
        for_actors
        {
            if ((*it)->getType() == ActorSprite::FLOOR_ITEM)
                continue;

            Being *being = static_cast<Being*>(*it);
            if (being->getType() == Being::PLAYER && being->getName() != "")
                names.push_back(being->getName());
        }
    }
};

class PlayerNPCNamesLister : public AutoCompleteLister
{
    void getAutoCompleteList(std::vector<std::string>& names) const
    {
        names.clear();

        const ActorSprites &mActors = actorSpriteManager->getAll();
        for_actors
        {
            if ((*it)->getType() == ActorSprite::FLOOR_ITEM)
                continue;

            Being *being = static_cast<Being*>(*it);
            if ((being->getType() == Being::PLAYER
                 || being->getType() == Being::NPC)
                && being->getName() != "")
                names.push_back(being->getName());
        }
    }
};

ActorSpriteManager::ActorSpriteManager()
{
    mPlayerNames = new PlayerNamesLister;
    mPlayerNPCNames = new PlayerNPCNamesLister;
}

ActorSpriteManager::~ActorSpriteManager()
{
    clear();
}

void ActorSpriteManager::setMap(Map *map)
{
    mMap = map;

    if (local_player)
        local_player->setMap(map);
}

void ActorSpriteManager::setPlayer(LocalPlayer *player)
{
    local_player = player;
    mActors.insert(player);
}

Being *ActorSpriteManager::createBeing(int id, ActorSprite::Type type, int subtype)
{
    Being *being = new Being(id, type, subtype, mMap);

    mActors.insert(being);
    return being;
}

FloorItem *ActorSpriteManager::createItem(int id, int itemId, const Vector &pos)
{
    FloorItem *floorItem = new FloorItem(id, itemId, pos, mMap);

    mActors.insert(floorItem);
    return floorItem;
}

void ActorSpriteManager::destroyActor(ActorSprite *actor)
{
    mActors.erase(actor);
    mDeleteActors.erase(actor);
    delete actor;
}

void ActorSpriteManager::scheduleDelete(ActorSprite *actor)
{
    if (!actor || actor == local_player)
        return;

    mDeleteActors.insert(actor);
}

Being *ActorSpriteManager::findBeing(int id) const
{
    for_actors
    {
        ActorSprite *actor = *it;
        if (actor->getId() == id &&
            actor->getType() != ActorSprite::FLOOR_ITEM)
            return static_cast<Being*>(actor);
    }

    return NULL;
}

Being *ActorSpriteManager::findBeing(int x, int y, ActorSprite::Type type) const
{
    beingFinder.x = x;
    beingFinder.y = y;
    beingFinder.type = type;

    ActorSpritesConstIterator it = find_if(mActors.begin(), mActors.end(),
                                           beingFinder);

    return (it == mActors.end()) ? NULL : static_cast<Being*>(*it);
}

Being *ActorSpriteManager::findBeingByPixel(int x, int y) const
{
    Map *map = Game::instance() ? Game::instance()->getCurrentMap() : 0;
    if (!map)
        return 0;

    const int halfTileHeight = map->getTileHeight() / 2;

    Being *closest = 0;
    int closestDist = 0;

    for_actors
    {
        if ((*it)->getType() == ActorSprite::FLOOR_ITEM)
            continue;

        Being *being = static_cast<Being*>(*it);

        const int halfWidth = std::max(16, being->getWidth() / 2);
        const int height = std::max(32, being->getHeight());
        const int halfHeight = height / 2;

        // Being sprites are drawn half a tile lower than they actually stand
        const int bottom = being->getPixelY() + halfTileHeight;

        const int dist = std::max(std::abs(bottom - halfHeight - y),
                                  std::abs(being->getPixelX() - x));

        if ((being->isAlive() &&
             being != local_player &&
             being->getPixelX() - halfWidth <= x &&
             being->getPixelX() + halfWidth >= x &&
             bottom - height <= y &&
             bottom >= y) &&
             (!closest || closestDist > dist))
        {
            closest = being;
            closestDist = dist;
        }
    }

    return closest;
}

FloorItem *ActorSpriteManager::findItem(int id) const
{
    for_actors
    {
        if ((*it)->getId() == id &&
            (*it)->getType() == ActorSprite::FLOOR_ITEM)
        {
            return static_cast<FloorItem*>(*it);
        }
    }

    return NULL;
}

FloorItem *ActorSpriteManager::findItem(int x, int y, int maxDist) const
{
    FloorItem *item = 0;
    int smallestDist = 0;
    for_actors
    {
        int dist = std::max(std::abs((*it)->getTileX() - x),
                            std::abs((*it)->getTileY() - y));
        if (((*it)->getType() == ActorSprite::FLOOR_ITEM) &&
            ((!item && dist <= maxDist) || dist < smallestDist))
        {
            item = static_cast<FloorItem*>(*it);
            smallestDist = dist;
        }
    }

    return item;
}

Being *ActorSpriteManager::findBeingByName(const std::string &name,
                                     ActorSprite::Type type) const
{
    for_actors
    {
        if ((*it)->getType() == ActorSprite::FLOOR_ITEM)
            continue;

        Being *being = static_cast<Being*>(*it);
        if (being->getName() == name &&
           (type == ActorSprite::UNKNOWN || type == being->getType()))
            return being;
    }
    return NULL;
}

const ActorSprites &ActorSpriteManager::getAll() const
{
    return mActors;
}

void ActorSpriteManager::logic()
{
    for_actors
        (*it)->logic();

    for (it = mDeleteActors.begin(), it_end = mDeleteActors.end();
         it != it_end; ++it)
    {
        mActors.erase(*it);
        delete *it;
    }

    mDeleteActors.clear();
}

void ActorSpriteManager::clear()
{
    if (local_player)
        mActors.erase(local_player);

    for_actors
        delete *it;
    mActors.clear();
    mDeleteActors.clear();

    if (local_player)
        mActors.insert(local_player);
}

Being *ActorSpriteManager::findNearestLivingBeing(int x, int y,
                                                  int maxTileDist,
                                                  ActorSprite::Type type,
                                                  Being *excluded) const
{
    Game *game = Game::instance();
    if (!game)
        return 0;

    Being *closestBeing = 0;
    int dist = 0;

    const int maxDist = maxTileDist * game->getCurrentTileWidth();

    for_actors
    {
        if ((*it)->getType() == ActorSprite::FLOOR_ITEM)
            continue;

        Being *being = static_cast<Being*>(*it);
        const Vector &pos = being->getPosition();
        int d = abs(((int) pos.x) - x) + abs(((int) pos.y) - y);

        if ((being->getType() == type || type == ActorSprite::UNKNOWN)
                && (d < dist || !closestBeing)  // it is closer
                && being->isAlive()             // no dead beings
                && being != excluded)
        {
            dist = d;
            closestBeing = being;
        }
    }

    return (maxDist >= dist) ? closestBeing : 0;
}

Being *ActorSpriteManager::findNearestLivingBeing(Being *aroundBeing,
                                                  int maxDist,
                                                  ActorSprite::Type type) const
{
    const Vector &pos = aroundBeing->getPosition();
    return findNearestLivingBeing((int)pos.x, (int)pos.y, maxDist, type,
                                  aroundBeing);
}

bool ActorSpriteManager::hasActorSprite(ActorSprite *actor) const
{
    for_actors
    {
        if (actor == *it)
            return true;
    }

    return false;
}

AutoCompleteLister *ActorSpriteManager::getPlayerNameLister()
{
    return mPlayerNames;
}

AutoCompleteLister *ActorSpriteManager::getPlayerNPCNameLister()
{
    return mPlayerNPCNames;
}

void ActorSpriteManager::updatePlayerNames()
{
    for_actors
    {
        if ((*it)->getType() == ActorSprite::FLOOR_ITEM)
            continue;

        Being *being = static_cast<Being*>(*it);
        if (being->getType() == ActorSprite::PLAYER && being->getName() != "")
            being->updateName();
    }
}
