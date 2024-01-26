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

#include "resources/specialdb.h"

#include "log.h"

#include "utils/dtor.h"


namespace
{
    SpecialInfos mSpecialInfos;
    bool mLoaded = false;
}

SpecialInfo::TargetMode SpecialDB::targetModeFromString(const std::string& str)
{
    if (str=="being")  return SpecialInfo::TARGET_BEING;
    else if (str=="point")  return SpecialInfo::TARGET_POINT;

    logger->log("SpecialDB: Warning, unknown target mode \"%s\"", str.c_str() );
    return SpecialInfo::TARGET_BEING;
}


void SpecialDB::init()
{
    if (mLoaded)
        unload();
}

void SpecialDB::readSpecialSetNode(xmlNodePtr node, const std::string &filename)
{
    std::string setName = XML::getProperty(node, "name", "Actions");

    for_each_xml_child_node(special, node)
    {
        if (xmlStrEqual(special->name, BAD_CAST "special"))
        {
            auto *info = new SpecialInfo();
            int id = XML::getProperty(special, "id", 0);
            info->id = id;
            info->set = setName;
            info->name = XML::getProperty(special, "name", "");
            info->icon = XML::getProperty(special, "icon", "");

            info->targetMode = targetModeFromString(XML::getProperty(special, "target", "being"));

            info->rechargeable = XML::getBoolProperty(special, "rechargeable", true);
            info->rechargeNeeded = 0;
            info->rechargeCurrent = 0;

            if (mSpecialInfos.find(id) != mSpecialInfos.end())
            {
                logger->log("SpecialDB: Duplicate special ID %d in %s, ignoring", id, filename.c_str());
            } else {
                mSpecialInfos[id] = info;
            }
        }
    }

}

void SpecialDB::checkStatus()
{
    mLoaded = true;
}


void SpecialDB::unload()
{

    delete_all(mSpecialInfos);
    mSpecialInfos.clear();

    mLoaded = false;
}


SpecialInfo *SpecialDB::get(int id)
{

    auto i = mSpecialInfos.find(id);

    if (i == mSpecialInfos.end())
    {
        return nullptr;
    }
    else
    {
        return i->second;
    }
    return nullptr;
}

