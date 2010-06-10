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

#include "actorspritemanager.h"

#include "localplayer.h"

#include "gui/viewport.h"

#include "utils/stringutils.h"
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
            Being* b = static_cast<Being*>(actor);

            Uint16 other_y = y + ((b->getType() == ActorSprite::NPC) ? 1 : 0);
            const Vector &pos = b->getPosition();
            return ((int) pos.x / 32 == x &&
                    ((int) pos.y / 32 == y || (int) pos.y / 32 == other_y) &&
                    b->isAlive() &&
                    (type == ActorSprite::UNKNOWN || b->getType() == type));
        }

        Uint16 x, y;
        ActorSprite::Type type;
} beingFinder;

ActorSpriteManager::ActorSpriteManager()
{
}

ActorSpriteManager::~ActorSpriteManager()
{
    clear();
}

void ActorSpriteManager::setMap(Map *map)
{
    mMap = map;

    if (player_node)
        player_node->setMap(map);
}

void ActorSpriteManager::setPlayer(LocalPlayer *player)
{
    player_node = player;
    mActors.push_back(player);
}

Being *ActorSpriteManager::createBeing(int id, ActorSprite::Type type, int subtype)
{
    Being *being = new Being(id, type, subtype, mMap);

    mActors.push_back(being);
    return being;
}

FloorItem *ActorSpriteManager::createItem(int id, int itemId, int x, int y)
{
    FloorItem *floorItem = new FloorItem(id, itemId, x, y, mMap);

    mActors.push_back(floorItem);
    return floorItem;
}

void ActorSpriteManager::destroy(ActorSprite *actor)
{
    mDeleteActors.push_back(actor);
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
    for_actors
    {
        if ((*it)->getType() == ActorSprite::FLOOR_ITEM)
            continue;

        Being *being = static_cast<Being*>(*it);

        int xtol = being->getWidth() / 2;
        int uptol = being->getHeight();

        if ((being->isAlive()) &&
            (being != player_node) &&
            (being->getPixelX() - xtol <= x) &&
            (being->getPixelX() + xtol >= x) &&
            (being->getPixelY() - uptol <= y) &&
            (being->getPixelY() >= y))
            return being;
    }

    return NULL;
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

FloorItem *ActorSpriteManager::findItem(int x, int y) const
{
    for_actors
    {
        if ((*it)->getTileX() == x && (*it)->getTileY() == y &&
            (*it)->getType() == ActorSprite::FLOOR_ITEM)
        {
            return static_cast<FloorItem*>(*it);
        }
    }

    return NULL;
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
        viewport->clearHover(*it);
        mActors.remove(*it);
        delete *it;
    }

    mDeleteActors.clear();
}

void ActorSpriteManager::clear()
{
    if (player_node)
    {
        player_node->setTarget(0);
        mActors.remove(player_node);
    }

    delete_all(mActors);
    mActors.clear();
    mDeleteActors.clear();

    if (player_node)
        mActors.push_back(player_node);
}

Being *ActorSpriteManager::findNearestLivingBeing(int x, int y,
                                                  int maxTileDist,
                                                  ActorSprite::Type type,
                                                  Being *excluded) const
{
    Being *closestBeing = 0;
    int dist = 0;

    const int maxDist = maxTileDist * 32;

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

void ActorSpriteManager::getPlayerNames(std::vector<std::string> &names,
                                        bool npcNames)
{
    names.clear();

    for_actors
    {
        if ((*it)->getType() == ActorSprite::FLOOR_ITEM)
            continue;

        Being *being = static_cast<Being*>(*it);
        if ((being->getType() == ActorSprite::PLAYER
             || (being->getType() == ActorSprite::NPC && npcNames))
            && being->getName() != "")
            names.push_back(being->getName());
    }
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
