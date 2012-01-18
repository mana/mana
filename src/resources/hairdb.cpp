/*
 *  Hair database
 *  Copyright (C) 2008  Aethyra Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "utils/xml.h"

#include <assert.h>

#define COLOR_WHITE "#ffffff"
#define HAIR_XML_FILE "hair.xml"

void HairDB::load()
{
    if (mLoaded)
        unload();

    // Default entry
    mHairColors[0] = COLOR_WHITE;

    XML::Document *doc = new XML::Document(HAIR_XML_FILE);
    xmlNodePtr root = doc->rootNode();

    if (!root || (!xmlStrEqual(root->name, BAD_CAST "colors")
        && !xmlStrEqual(root->name, BAD_CAST "hair")))
    {
        logger->log("HairDb: Failed to find any old <colors> or new "
                    "<hair> nodes.");
        delete doc;
        mLoaded = true;
        return;
    }

    // Old colors.xml file style. The hair style will be declared
    // in the items.xml file.
    if (xmlStrEqual(root->name, BAD_CAST "colors"))
    {
        loadHairColorsNode(root);
    }
    else if (xmlStrEqual(root->name, BAD_CAST "hair"))
    {
        // Loading new format: hair styles + colors.
        for_each_xml_child_node(node, root)
        {
            if (xmlStrEqual(node->name, BAD_CAST "styles"))
            {
                loadHairStylesNode(root);
            }
            else if (xmlStrEqual(node->name, BAD_CAST "colors"))
            {
                loadHairColorsNode(node);
            }
        }
    }

    delete doc;

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

void HairDB::loadHairColorsNode(xmlNodePtr colorsNode)
{
    for_each_xml_child_node(node, colorsNode)
    {
        if (xmlStrEqual(node->name, BAD_CAST "color"))
        {
            int id = XML::getProperty(node, "id", 0);

            if (mHairColors.find(id) != mHairColors.end())
                logger->log("HairDb: Redefinition of color Id %d", id);

            mHairColors[id] = XML::getProperty(node, "value", COLOR_WHITE);
        }
    }
}

void HairDB::loadHairStylesNode(xmlNodePtr stylesNode)
{
    // TODO: Add support of the races.xml file.
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

const std::string &HairDB::getHairColor(int id)
{
    if (!mLoaded)
        load();

    ColorConstIterator it = mHairColors.find(id);
    if (it != mHairColors.end())
        return it->second;

    logger->log("HairDb: Error, unknown color Id# %d", id);
    return mHairColors[0];
}

std::vector<int> HairDB::getHairStyleIds(int maxId) const
{
    std::vector<int> hairStylesIds;
    for (HairStylesConstIterator it = mHairStyles.begin(),
        it_end = mHairStyles.end(); it != it_end; ++it)
    {
        // Don't give ids higher than the requested maximum.
        if (maxId > 0 && (*it) > maxId)
            continue;
        hairStylesIds.push_back(*it);
    }
    return hairStylesIds;
}

std::vector<int> HairDB::getHairColorIds(int maxId) const
{
    std::vector<int> hairColorsIds;
    for (ColorConstIterator it = mHairColors.begin(),
        it_end = mHairColors.end(); it != it_end; ++it)
    {
        // Don't give ids higher than the requested maximum.
        if (maxId > 0 && it->first > maxId)
            continue;
        hairColorsIds.push_back(it->first);
    }
    return hairColorsIds;
}
