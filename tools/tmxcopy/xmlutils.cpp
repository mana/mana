/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xmlutils.h"

namespace XML
{
    int
    getProperty(xmlNodePtr node, const char* name, int def)
    {
        int &ret = def;

        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop) {
            ret = atoi((char*)prop);
            xmlFree(prop);
        }

        return ret;
    }

    double
    getFloatProperty(xmlNodePtr node, const char* name, double def)
    {
        double &ret = def;

        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop) {
            ret = atof((char*)prop);
            xmlFree(prop);
        }

        return ret;
    }

    std::string
    getProperty(xmlNodePtr node, const char *name, const std::string &def)
    {
        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop) {
            std::string val = (char*)prop;
            xmlFree(prop);
            return val;
        }

        return def;
    }

    xmlNodePtr findFirstChildByName(xmlNodePtr parent, const char *name)
    {
        for_each_xml_child_node(child, parent)
            if (xmlStrEqual(child->name, BAD_CAST name))
                return child;

        return NULL;
    }
}
