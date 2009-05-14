/*
 *  Color database
 *  Copyright (C) 2008  Aethyra Development Team
 *
 *  This file is part of The Mana World.
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

#include "resources/colordb.h"

#include "log.h"

#include "utils/xml.h"

#include <libxml/tree.h>

namespace
{
    ColorDB::Colors mColors;
    bool mLoaded = false;
    std::string mFail = "#ffffff";
}

void ColorDB::load()
{
    if (mLoaded)
        return;

    XML::Document *doc = new XML::Document("hair.xml");
    xmlNodePtr root = doc->rootNode();
    bool hairXml = true;

    if (!root || !xmlStrEqual(root->name, BAD_CAST "colors"))
    {
        logger->log("Trying to fall back on colors.xml");

        hairXml = false;

        delete doc;
        doc = new XML::Document("colors.xml");
        root = doc->rootNode();

        if (!root || !xmlStrEqual(root->name, BAD_CAST "colors"))
        {
            logger->log("ColorDB: Failed to find any color files.");
            mColors[0] = mFail;
            mLoaded = true;

            delete doc;

            return;
        }
    }
    for_each_xml_child_node(node, root)
    {
        if (xmlStrEqual(node->name, BAD_CAST "color"))
        {
            int id = XML::getProperty(node, "id", 0);

            if (mColors.find(id) != mColors.end())
            {
                logger->log("ColorDB: Redefinition of dye ID %d", id);
            }

            mColors[id] = hairXml ? XML::getProperty(node, "value", "#FFFFFF") :
                                    XML::getProperty(node, "dye", "#FFFFFF");
        }
    }

    delete doc;

    mLoaded = true;
}

void ColorDB::unload()
{
    logger->log("Unloading color database...");

    mColors.clear();
    mLoaded = false;
}

std::string &ColorDB::get(int id)
{
    if (!mLoaded)
        load();

    ColorIterator i = mColors.find(id);

    if (i == mColors.end())
    {
        logger->log("ColorDB: Error, unknown dye ID# %d", id);
        return mFail;
    }
    else
    {
        return i->second;
    }
}

int ColorDB::size()
{
    return mColors.size();
}
