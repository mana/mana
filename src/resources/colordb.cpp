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
#include "../utils/xml.h"

#define HAIR_COLOR_FILE "colors.xml"

namespace
{
    ColorDB::Colors mColors;
    bool mLoaded = false;
    std::string mFail = "";
}

void ColorDB::load()
{
    if (mLoaded)
    {
        return;
    }

    XML::Document doc(HAIR_COLOR_FILE);
    xmlNodePtr root = doc.rootNode();

    if (!root || !xmlStrEqual(root->name, BAD_CAST "colors"))
    {
        logger->log("Error loading colors file: "
                    HAIR_COLOR_FILE);

        // Provide "legacy" support for the TMW server that
        // doesn't seperate out hair colors from the actual
        // code. Seriously, this is freaking annoying.
        mColors[0] = "#8c4b41,da9041,ffffff"; // light brown
        mColors[1] = "#06372b,489e25,fdedcc"; // green
        mColors[2] = "#5f0b33,91191c,f9ad81"; // dark red
        mColors[3] = "#602486,934cc3,fdc689"; // purple
        mColors[4] = "#805e74,c6b09b,ffffff"; // white
        mColors[5] = "#8c6625,dab425,ffffff"; // yellow
        mColors[6] = "#1d2d6d,1594a3,fdedcc"; // blue
        mColors[7] = "#831f2d,be4f2d,f8cc8b"; // brown
        mColors[8] = "#432482,584bbc,dae8e5"; // light blue
        mColors[9] = "#460850,611967,e7b4ae"; // dark purple
    }
    else
    {
        for_each_xml_child_node(node, root)
        {
            if (xmlStrEqual(node->name, BAD_CAST "color"))
            {
                int id = XML::getProperty(node, "id", 0);

                if (mColors.find(id) != mColors.end())
                {
                    logger->log("ColorDB: Redefinition of dye ID %d", id);
                }

                mColors[id] = XML::getProperty(node, "dye", "");
            }
        }
    }

    mLoaded = true;
}

void ColorDB::unload()
{
    logger->log("Unloading color database...");

    mColors.clear();
    mLoaded = false;
}

std::string& ColorDB::get(int id)
{
    if(!mLoaded)
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
