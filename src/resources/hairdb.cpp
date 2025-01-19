/*
 *  Hair database
 *  Copyright (C) 2008  Aethyra Development Team
 *  Copyright (C) 2009-2013  The Mana Developers
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

#include "resources/hairdb.h"

#include "log.h"

#define COLOR_WHITE "#ffffff"


void HairDB::init()
{
    if (mLoaded)
        unload();

    // Default entry
    mHairColors[0] = COLOR_WHITE;
}

void HairDB::readHairColorNode(XML::Node node, const std::string &filename)
{
    int id = node.getProperty("id", 0);

    if (mHairColors.find(id) != mHairColors.end())
        logger->log("HairDb: Redefinition of color Id %d in %s", id, filename.c_str());

    mHairColors[id] = node.getProperty("value", COLOR_WHITE);
}

void HairDB::checkStatus()
{
    mLoaded = true;
}

void HairDB::unload()
{
    if (!mLoaded)
        return;

    logger->log("Unloading hair style and color database...");

    mHairColors.clear();
    mHairStyles.clear();

    mLoaded = false;
}

void HairDB::addHairStyle(int id)
{
    // TODO: Adapt the sprite handling with hair styles separated from items.
    // And remove that hack for negative ids.
    if (id < 0)
        id = -id;

    if (mHairStyles.find(id) != mHairStyles.end())
        logger->log("Warning: Redefinition of hairstyle id %i:", id);

    mHairStyles.insert(id);
}

const std::string &HairDB::getHairColor(int id) const
{
    if (!mLoaded)
    {
        // no idea if this can happen, but that check existed before
        logger->log("WARNING: HairDB::getHairColor() called before settings were loaded!");
    }
    auto it = mHairColors.find(id);
    if (it != mHairColors.end())
        return it->second;

    logger->log("HairDb: Error, unknown color Id# %d", id);
    return mHairColors.at(0);
}

std::vector<int> HairDB::getHairStyleIds(int maxId) const
{
    std::vector<int> hairStylesIds;
    for (int hairStyle : mHairStyles)
    {
        // Don't give ids higher than the requested maximum.
        if (maxId > 0 && hairStyle > maxId)
            continue;
        hairStylesIds.push_back(hairStyle);
    }
    return hairStylesIds;
}

std::vector<int> HairDB::getHairColorIds(int minId, int maxId) const
{
    std::vector<int> hairColorsIds;
    for (const auto &[id, _] : mHairColors)
    {
        // Don't give ids higher than the requested maximum.
        if ((maxId > 0 && id > maxId) || id < minId)
            continue;
        hairColorsIds.push_back(id);
    }
    return hairColorsIds;
}
