/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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
