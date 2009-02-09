/*
 *  Aethyra
 *  Copyright 2008 Aethyra Development Team
 *
 *  This file is part of Aethyra.
 *
 *  Aethyra is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  Aethyra is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Aethyra; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <cassert>
#include <libxml/tree.h>

#include "colordb.h"

#include "../log.h"

#include "../utils/dtor.h"
#include "../utils/gettext.h"
#include "../utils/xml.h"

#define HAIR_COLOR_FILE "colors.xml"
#define TMW_COLOR_FILE "hair.xml"

namespace
{
    ColorDB::Colors mColors;
    bool mLoaded = false;
    std::string mFail = "#ffffff";
}

void ColorDB::load()
{
    if (mLoaded)
    {
        return;
    }

    XML::Document *doc = new XML::Document(HAIR_COLOR_FILE);
    xmlNodePtr root = doc->rootNode();
    bool TMWHair = false;

    if (!root || !xmlStrEqual(root->name, BAD_CAST "colors"))
    {
        logger->log(_("Trying TMW's color file, %s."), TMW_COLOR_FILE);

        TMWHair = true;

        delete doc;

        doc = new XML::Document(TMW_COLOR_FILE);
        root = doc->rootNode();
        if (!root || !xmlStrEqual(root->name, BAD_CAST "colors"))
        {
            logger->log(_("ColorDB: Failed"));
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
                logger->log(_("ColorDB: Redefinition of dye ID %d"), id);
            }

            TMWHair ? mColors[id] = XML::getProperty(node, "value", "#FFFFFF") :
                      mColors[id] = XML::getProperty(node, "dye", "#FFFFFF");
        }
    }

    delete doc;

    mLoaded = true;
}

void ColorDB::unload()
{
    logger->log(_("Unloading color database..."));

    mColors.clear();
    mLoaded = false;
}

std::string& ColorDB::get(int id)
{
    if (!mLoaded)
        load();

    ColorIterator i = mColors.find(id);

    if (i == mColors.end())
    {
        logger->log(_("ColorDB: Error, unknown dye ID# %d"), id);
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
