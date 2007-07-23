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

#include "beingmanager.h"

#include "localplayer.h"
#include "monster.h"
#include "npc.h"
#include "player.h"

#include "utils/dtor.h"

class FindBeingFunctor
{
    public:
        bool operator() (Being *being)
        {
            Uint16 other_y = y + ((being->getType() == Being::NPC) ? 1 : 0);
            return (being->mX / 32 == x &&
                    (being->mY / 32 == y || being->mY / 32 == other_y) &&
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

Being* BeingManager::createBeing(Uint16 id, Uint16 job)
{
    Being *being;

    if (job < 10)
        being = new Player(id, job, mMap);
    else if (job >= 100 & job < 200)
        being = new NPC(id, job, mMap);
    else if (job >= 1000 && job < 1200)
        being = new Monster(id, job, mMap);
    else
        being = new Being(id, job, mMap);

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

    for_each(mBeings.begin(), mBeings.end(), make_dtor(mBeings));
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
        int d = abs(being->mX - x) + abs(being->mY - y);

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
