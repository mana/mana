/*
 *  Character creation settings
 *  Copyright (C) 2011-2013  The ManaPlus Developers
 *  Copyright (C) 2013  The Mana Developers
 *
 *  This file is part of The ManaPlus Client.
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

#include "resources/chardb.h"

#include "log.h"

#include "utils/xml.h"

namespace
{
    bool mLoaded = false;
    unsigned mMinHairColor = 0;
    unsigned mMaxHairColor = 0;
    unsigned mMinHairStyle = 0;
    unsigned mMaxHairStyle = 0;
    unsigned mMinStat = 0;
    unsigned mMaxStat = 0;
    unsigned mSumStat = 0;
    std::vector<int> mDefaultItems;
}

static void loadMinMax(XML::Node node, unsigned *min, unsigned *max)
{
    *min = node.getProperty("min", 1);
    *max = node.getProperty("max", 10);
}

void CharDB::load()
{
    if (mLoaded)
        unload();

    XML::Document doc("charcreation.xml");
    XML::Node root = doc.rootNode();

    if (!root || root.name() != "chars")
    {
        logger->log("CharDB: Failed to parse charcreation.xml.");
        return;
    }

    for (auto node : root.children())
    {
        if (node.name() == "haircolor")
        {
            loadMinMax(node, &mMinHairColor, &mMaxHairColor);
        }
        else if (node.name() == "hairstyle")
        {
            loadMinMax(node, &mMinHairStyle, &mMaxHairStyle);
        }
        else if (node.name() == "stat")
        {
            loadMinMax(node, &mMinStat, &mMaxStat);
            mSumStat = node.getProperty("sum", 0);
        }
        else if (node.name() == "item")
        {
            const int id = node.getProperty("id", 0);
            if (id > 0)
                mDefaultItems.push_back(id);
        }
    }

    mLoaded = true;
}

void CharDB::unload()
{
    logger->log("Unloading chars database...");

    mLoaded = false;
}

unsigned CharDB::getMinHairColor()
{
    return mMinHairColor;
}

unsigned CharDB::getMaxHairColor()
{
    return mMaxHairColor;
}

unsigned CharDB::getMinHairStyle()
{
    return mMinHairStyle;
}

unsigned CharDB::getMaxHairStyle()
{
    return mMaxHairStyle;
}

unsigned CharDB::getMinStat()
{
    return mMinStat;
}

unsigned CharDB::getMaxStat()
{
    return mMaxStat;
}

unsigned CharDB::getSumStat()
{
    return mSumStat;
}

const std::vector<int> &CharDB::getDefaultItems()
{
    return mDefaultItems;
}
