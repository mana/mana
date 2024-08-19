/*
 *  The Mana Client
 *  Copyright (C) 2010-2013  The Mana Developers
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

#include "resources/abilitydb.h"

#include "log.h"

#include "utils/dtor.h"

#include <map>

namespace
{
    std::map<int, AbilityInfo *> mAbilityInfos;
    bool mLoaded = false;
}

AbilityInfo::TargetMode AbilityDB::targetModeFromString(const std::string& str)
{
    if (str == "being")
        return AbilityInfo::TARGET_BEING;
    if (str == "point")
        return AbilityInfo::TARGET_POINT;

    logger->log("AbilityDB: Warning, unknown target mode \"%s\"", str.c_str() );
    return AbilityInfo::TARGET_BEING;
}


void AbilityDB::init()
{
    if (mLoaded)
        unload();
}

void AbilityDB::readAbilitySetNode(XML::Node node, const std::string &filename)
{
    std::string setName = node.getProperty("name", "Actions");

    for (auto ability : node.children())
    {
        if (ability.name() == "ability")
        {
            auto *info = new AbilityInfo();
            int id = ability.getProperty("id", 0);
            info->id = id;
            info->set = setName;
            info->name = ability.getProperty("name", "");
            info->icon = ability.getProperty("icon", "");

            info->targetMode = targetModeFromString(ability.getProperty("target", "being"));

            info->rechargeable = ability.getBoolProperty("rechargeable", true);
            info->rechargeNeeded = 0;
            info->rechargeCurrent = 0;

            if (mAbilityInfos.find(id) != mAbilityInfos.end())
            {
                logger->log("AbilityDB: Duplicate ability ID %d in %s, ignoring", id, filename.c_str());
            } else {
                mAbilityInfos[id] = info;
            }
        }
    }

}

void AbilityDB::checkStatus()
{
    mLoaded = true;
}


void AbilityDB::unload()
{

    delete_all(mAbilityInfos);
    mAbilityInfos.clear();

    mLoaded = false;
}


AbilityInfo *AbilityDB::get(int id)
{

    auto i = mAbilityInfos.find(id);

    if (i == mAbilityInfos.end())
    {
        return nullptr;
    }
    else
    {
        return i->second;
    }
    return nullptr;
}
