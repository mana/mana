/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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
#include "utils/xml.h"


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

void SpecialDB::load()
{
    if (mLoaded)
        unload();

    logger->log("Initializing special database...");

    XML::Document doc("specials.xml");
    xmlNodePtr root = doc.rootNode();

    if (!root || !xmlStrEqual(root->name, BAD_CAST "specials"))
    {
        logger->log("Error loading specials file specials.xml");
        return;
    }

    std::string setName;

    for_each_xml_child_node(set, root)
    {
        if (xmlStrEqual(set->name, BAD_CAST "set"))
        {
            setName = XML::getProperty(set, "name", "Actions");


            for_each_xml_child_node(special, set)
            {
                if (xmlStrEqual(special->name, BAD_CAST "special"))
                {
                    SpecialInfo *info = new SpecialInfo();
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
                        logger->log("SpecialDB: Duplicate special ID %d (ignoring)", id);
                    } else {
                        mSpecialInfos[id] = info;
                    }
                }
            }
        }
    }

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

    SpecialInfos::iterator i = mSpecialInfos.find(id);

    if (i == mSpecialInfos.end())
    {
        return NULL;
    }
    else
    {
        return i->second;
    }
    return NULL;
}

