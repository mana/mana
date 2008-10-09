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

#include <cassert>

#include "beingmanager.h"

#include "localplayer.h"
#include "monster.h"
#include "npc.h"
#include "player.h"

#include "net/protocol.h"
#include "utils/dtor.h"

class FindBeingFunctor
{
    public:
        bool operator() (Being *being)
        {
            Uint16 other_y = y + ((being->getType() == Being::NPC) ? 1 : 0);
            const Vector &pos = being->getPosition();
            return ((int) pos.x / 32 == x &&
                    ((int) pos.y / 32 == y || (int) pos.y / 32 == other_y) &&
                    being->mAction != Being::DEAD &&
                    (type == Being::UNKNOWN || being->getType() == type));
        }

        Uint16 x, y;
        Being::Type type;
} beingFinder;

void BeingManager::setMap(Map *map)
{
    mMap = map;
    if (player_node)
        player_node->setMap(map);
}

void BeingManager::setPlayer(LocalPlayer *player)
{
    player_node = player;
    mBeings.push_back(player);
}

Being* BeingManager::createBeing(int id, int type, int subtype)
{
    Being *being;

    switch (type)
    {
        case OBJECT_PLAYER:
            being = new Player(id, subtype, mMap);
            break;
        case OBJECT_NPC:
            being = new NPC(id, subtype, mMap);
            break;
        case OBJECT_MONSTER:
            being = new Monster(id, subtype, mMap);
            break;
        default:
            assert(false);
    }

    mBeings.push_back(being);
    return being;
}

void BeingManager::destroyBeing(Being *being)
{
    mBeings.remove(being);
    delete being;
}

Being* BeingManager::findBeing(Uint16 id)
{
    for (BeingIterator i = mBeings.begin(); i != mBeings.end(); i++)
    {
        Being *being = (*i);
        if (being->getId() == id) {
            return being;
        }
    }
    return NULL;
}

Being* BeingManager::findBeing(Uint16 x, Uint16 y, Being::Type type)
{
    beingFinder.x = x;
    beingFinder.y = y;
    beingFinder.type = type;

    BeingIterator i = find_if(mBeings.begin(), mBeings.end(), beingFinder);

    return (i == mBeings.end()) ? NULL : *i;
}

Beings& BeingManager::getAll()
{
    return mBeings;
}

void BeingManager::logic()
{
    BeingIterator i = mBeings.begin();
    while (i != mBeings.end())
    {
        Being *being = (*i);

        being->logic();

        /*
        if (being->mAction == Being::DEAD && being->mFrame >= 20)
        {
            delete being;
            i = mBeings.erase(i);
        }
        else*/ {
            i++;
        }
    }
}

void BeingManager::clear()
{
    if (player_node)
    {
        mBeings.remove(player_node);
    }

    delete_all(mBeings);
    mBeings.clear();

    if (player_node)
    {
        mBeings.push_back(player_node);
    }
}

Being* BeingManager::findNearestLivingBeing(Uint16 x, Uint16 y, int maxdist,
                                            Being::Type type)
{
    Being *closestBeing = NULL;
    int dist = 0;

    for (BeingIterator i = mBeings.begin(); i != mBeings.end(); i++)
    {
        Being *being = (*i);
        const Vector &pos = being->getPosition();
        int d = abs((int) pos.x - x) + abs((int) pos.y - y);

        if ((being->getType() == type || type == Being::UNKNOWN)
                && (d < dist || closestBeing == NULL)   // it is closer
                && being->mAction != Being::DEAD        // no dead beings
           )
        {
            dist = d;
            closestBeing = being;
        }
    }

    return (maxdist >= dist) ? closestBeing : NULL;
}

Being* BeingManager::findNearestLivingBeing(Being *aroundBeing, int maxdist,
                                            Being::Type type)
{
    Being *closestBeing = NULL;
    int dist = 0;
    const Vector &aroundBeingPos = aroundBeing->getPosition();

    for (BeingIterator i = mBeings.begin(); i != mBeings.end(); i++)
    {
        Being *being = (*i);
        const Vector &pos = being->getPosition();
        int d = abs((int) pos.x - aroundBeingPos.x) +
                abs((int) pos.y - aroundBeingPos.y);

        if ((being->getType() == type || type == Being::UNKNOWN)
                && (d < dist || closestBeing == NULL)   // it is closer
                && being->mAction != Being::DEAD        // no dead beings
                && being != aroundBeing
           )
        {
            dist = d;
            closestBeing = being;
        }
    }

    return (maxdist >= dist) ? closestBeing : NULL;
}
