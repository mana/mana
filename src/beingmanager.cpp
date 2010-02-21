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

#include "beingmanager.h"

#include "localplayer.h"
#include "monster.h"
#include "npc.h"
#include "player.h"

#include "net/gamehandler.h"
#include "net/net.h"

#include "utils/stringutils.h"
#include "utils/dtor.h"

#include <cassert>

class FindBeingFunctor
{
    public:
        bool operator() (Being *being)
        {
            Uint16 other_y = y + ((being->getType() == Being::NPC) ? 1 : 0);
            const Vector &pos = being->getPosition();
            return ((int) pos.x / 32 == x &&
                    ((int) pos.y / 32 == y || (int) pos.y / 32 == other_y) &&
                    being->isAlive() &&
                    (type == Being::UNKNOWN || being->getType() == type));
        }

        Uint16 x, y;
        Being::Type type;
} beingFinder;

BeingManager::BeingManager()
{
}

BeingManager::~BeingManager()
{
    clear();
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

Being *BeingManager::createBeing(int id, Being::Type type, int subtype)
{
    Being *being;

    switch (type)
    {
        case Being::PLAYER:
            being = new Player(id, subtype, mMap);
            break;
        case Being::NPC:
            being = new NPC(id, subtype, mMap);
            break;
        case Being::MONSTER:
            being = new Monster(id, subtype, mMap);
            break;
        case Being::UNKNOWN:
            being = new Being(id, subtype, mMap);
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

Being *BeingManager::findBeing(int id) const
{
    for (Beings::const_iterator i = mBeings.begin(), i_end = mBeings.end();
         i != i_end; ++i)
    {
        Being *being = (*i);
        if (being->getId() == id)
            return being;
    }
    return NULL;
}

Being *BeingManager::findBeing(int x, int y, Being::Type type) const
{
    beingFinder.x = x;
    beingFinder.y = y;
    beingFinder.type = type;

    Beings::const_iterator i = find_if(mBeings.begin(), mBeings.end(),
                                       beingFinder);

    return (i == mBeings.end()) ? NULL : *i;
}

Being *BeingManager::findBeingByPixel(int x, int y) const
{
    Beings::const_iterator itr = mBeings.begin();
    Beings::const_iterator itr_end = mBeings.end();

    for (; itr != itr_end; ++itr)
    {
        Being *being = (*itr);

        int xtol = being->getWidth() / 2;
        int uptol = being->getHeight();

        if ((being->isAlive()) &&
            (being != player_node) &&
            (being->getPixelX() - xtol <= x) &&
            (being->getPixelX() + xtol >= x) &&
            (being->getPixelY() - uptol <= y) &&
            (being->getPixelY() >= y))
        {
            return being;
        }
    }

    return NULL;
}

Being *BeingManager::findBeingByName(const std::string &name,
                                     Being::Type type) const
{
    for (Beings::const_iterator i = mBeings.begin(), i_end = mBeings.end();
         i != i_end; ++i)
    {
        Being *being = (*i);
        if (being->getName() == name &&
           (type == Being::UNKNOWN || type == being->getType()))
            return being;
    }
    return NULL;
}

const Beings &BeingManager::getAll() const
{
    return mBeings;
}

void BeingManager::logic()
{
    Beings::iterator i = mBeings.begin();
    while (i != mBeings.end())
    {
        Being *being = (*i);

        being->logic();

        if (!being->isAlive() &&
            Net::getGameHandler()->removeDeadBeings() &&
            being->getCurrentFrame() >= 20)
        {
            delete being;
            i = mBeings.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

void BeingManager::clear()
{
    if (player_node)
        mBeings.remove(player_node);

    delete_all(mBeings);
    mBeings.clear();

    if (player_node)
        mBeings.push_back(player_node);
}

Being *BeingManager::findNearestLivingBeing(int x, int y,
                                            int maxTileDist,
                                            Being::Type type) const
{
    Being *closestBeing = 0;
    int dist = 0;

    const int maxDist = maxTileDist * 32;

    Beings::const_iterator itr = mBeings.begin();
    Beings::const_iterator itr_end = mBeings.end();

    for (; itr != itr_end; ++itr)
    {
        Being *being = (*itr);
        const Vector &pos = being->getPosition();
        int d = abs(((int) pos.x) - x) + abs(((int) pos.y) - y);

        if ((being->getType() == type || type == Being::UNKNOWN)
                && (d < dist || !closestBeing)          // it is closer
                && being->isAlive())       // no dead beings
        {
            dist = d;
            closestBeing = being;
        }
    }

    return (maxDist >= dist) ? closestBeing : 0;
}

Being *BeingManager::findNearestLivingBeing(Being *aroundBeing, int maxDist,
                                            Being::Type type) const
{
    const Vector &pos = aroundBeing->getPosition();
    return findNearestLivingBeing((int)pos.x, (int)pos.y, maxDist, type);
}

bool BeingManager::hasBeing(Being *being) const
{
    for (Beings::const_iterator i = mBeings.begin(), i_end = mBeings.end();
         i != i_end; ++i)
    {
        if (being == *i)
            return true;
    }

    return false;
}

void BeingManager::getPlayerNames(std::vector<std::string> &names,
                                  bool npcNames)
{
    Beings::iterator i = mBeings.begin();
    names.clear();

    while (i != mBeings.end())
    {
        Being *being = (*i);
        if ((being->getType() == Being::PLAYER
             || (being->getType() == Being::NPC && npcNames))
            && being->getName() != "")
        {
            names.push_back(being->getName());
        }
        ++i;
    }
}
