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

#ifndef _XMLUTILS_H
#define _XMLUTILS_H

#include <libxml/tree.h>

#include <string>

/**
 * XML helper functions.
 */
namespace XML
{
    /**
     * Gets an integer property from an xmlNodePtr.
     */
    int
    getProperty(xmlNodePtr node, const char *name, int def);

    /**
     * Gets an floating point property from an xmlNodePtr.
     */
    double
    getFloatProperty(xmlNodePtr node, const char *name, double def);

    /**
     * Gets a string property from an xmlNodePtr.
     */
    std::string
    getProperty(xmlNodePtr node, const char *name, const std::string &def);

    /**
     * Finds the first child node with the given name
     */
    xmlNodePtr findFirstChildByName(xmlNodePtr parent, const char *name);
}

#define for_each_xml_child_node(var, parent) \
    for (xmlNodePtr var = parent->xmlChildrenNode; var; var = var->next)

#endif
