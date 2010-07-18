/*
 *  The Mana Client
 *  Copyright (C) 2010  The Mana Developers
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

#include "playerinfo.h"

#include "event.h"
#include "eventmanager.h"

PlayerInfoBackend PlayerInfo::mData;

void PlayerInfo::setBackend(const PlayerInfoBackend &backend)
{
    mData = backend;
}

// ------------------------------- Attributes --------------------------------------

int PlayerInfo::getAttribute(int id)
{
    IntMap::const_iterator it = mData.mAttributes.find(id);
    if (it != mData.mAttributes.end())
        return it->second;
    else
        return 0;
}

void PlayerInfo::setAttribute(int id, int value, bool notify)
{
    mData.mAttributes[id] = value;
    if (notify)
        triggerAttr(id);
}

// ------------------------------- Stats --------------------------------------

int PlayerInfo::getStatBase(int id)
{
    StatMap::const_iterator it = mData.mStats.find(id);
    if (it != mData.mStats.end())
        return it->second.base;
    else
        return 0;
}

void PlayerInfo::setStatBase(int id, int value, bool notify)
{
    mData.mStats[id].base = value;
    if (notify)
        triggerStat(id);
}

int PlayerInfo::getStatMod(int id)
{

    StatMap::const_iterator it = mData.mStats.find(id);
    if (it != mData.mStats.end())
        return it->second.mod;
    else
        return 0;
}

void PlayerInfo::setStatMod(int id, int value, bool notify)
{
    mData.mStats[id].mod = value;
    if (notify)
        triggerStat(id);
}

int PlayerInfo::getStatEffective(int id)
{
    StatMap::const_iterator it = mData.mStats.find(id);
    if (it != mData.mStats.end())
        return it->second.base + it->second.mod;
    else
        return 0;
}

std::pair<int, int> PlayerInfo::getStatExperience(int id)
{
    StatMap::const_iterator it = mData.mStats.find(id);
    int a, b;
    if (it != mData.mStats.end())
    {
        a = it->second.exp;
        b = it->second.expneed;
    }
    else
    {
        a = 0;
        b = 0;
    }
    return std::pair<int, int>(a, b);
}

void PlayerInfo::setStatExperience(int id, int have, int need, bool notify)
{
    mData.mStats[id].exp = have;
    mData.mStats[id].expneed = need;
    if (notify)
        triggerStat(id);
}

// ------------------------------- Triggers --------------------------------------

void PlayerInfo::triggerAttr(int id)
{
    Mana::Event event("UpdateAttribute");
    event.setInt("id", id);
    event.setInt("value", mData.mAttributes.find(id)->second);
    Mana::EventManager::trigger("Attributes", event);
}

void PlayerInfo::triggerStat(int id)
{
    StatMap::iterator it = mData.mStats.find(id);
    Mana::Event event("UpdateStat");
    event.setInt("id", id);
    event.setInt("base", it->second.base);
    event.setInt("mod", it->second.mod);
    event.setInt("exp", it->second.exp);
    event.setInt("expneeded", it->second.expneed);
    Mana::EventManager::trigger("Attributes", event);
}
