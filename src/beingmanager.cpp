/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#include "beingmanager.h"
#include "localplayer.h"
#include "monster.h"
#include "npc.h"
#include "player.h"

#include "net/messageout.h"
#include "net/protocol.h"

#include "utils/dtor.h"

class FindBeingFunctor
{
    public:
        bool operator() (Being *being)
        {
            Uint16 other_y = y + ((being->getType() == Being::NPC) ? 1 : 0);
            return (being->mX == x &&
                    (being->mY == y || being->mY == other_y) &&
                    being->mAction != Being::DEAD &&
                    (type == Being::UNKNOWN || being->getType() == type));
        }

        Uint16 x, y;
        Being::Type type;
} beingFinder;

BeingManager::BeingManager(Network *network):
    mNetwork(network)
{
}

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

Being* BeingManager::createBeing(Uint32 id, Uint16 job)
{
    Being *being;

    if (job < 10)
        being = new Player(id, job, mMap);
    else if (job >= 100 && job < 200)
        being = new NPC(id, job, mMap, mNetwork);
    else if (job >= 1000 && job < 1200)
        being = new Monster(id, job, mMap);
    else
        being = new Being(id, job, mMap);

    // Player or NPC
    if (job < 200)
    {
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(0x0094);
        outMsg.writeInt32(id);//readLong(2));
    }

    mBeings.push_back(being);

    return being;
}

void BeingManager::destroyBeing(Being *being)
{
    mBeings.remove(being);
    delete being;
}

Being* BeingManager::findBeing(Uint32 id)
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

Being* BeingManager::findBeingByPixel(Uint16 x, Uint16 y)
{
    BeingIterator itr = mBeings.begin();
    BeingIterator itr_end = mBeings.end();

    for (; itr != itr_end; ++itr)
    {
        Being *being = (*itr);

        int xtol = being->getWidth();
        int uptol = being->getHeight() / 2;

        if ((being->mAction != Being::DEAD) &&
            (being != player_node) &&
            (being->getPixelX() <= x) &&
            (being->getPixelX() + xtol >= x) &&
            (being->getPixelY() - uptol <= y) &&
            (being->getPixelY() + uptol >= y))
        {
            return being;
        }
    }

    return NULL;
}

Being* BeingManager::findBeingByName(std::string name, Being::Type type)
{
    for (BeingIterator i = mBeings.begin(); i != mBeings.end(); i++)
    {
        Being *being = (*i);
        if (being->getName() == name
            && (type == Being::UNKNOWN
                || type == being->getType()))
            return being;
    }
    return NULL;
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

        if (being->mAction == Being::DEAD && being->mFrame >= 20)
        {
            delete being;
            i = mBeings.erase(i);
        }
        else {
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

    BeingIterator itr = mBeings.begin();
    BeingIterator itr_end = mBeings.end();

    for (; itr != itr_end; ++itr)
    {
        Being *being = (*itr);
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

Being* BeingManager::findNearestLivingBeing(Being *aroundBeing, int maxdist,
                                            Being::Type type)
{
    Being *closestBeing = NULL;
    int dist = 0;
    int x = aroundBeing->mX;
    int y = aroundBeing->mY;

    for (BeingIterator i = mBeings.begin(); i != mBeings.end(); i++)
    {
        Being *being = (*i);
        int d = abs(being->mX - x) + abs(being->mY - y);

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
