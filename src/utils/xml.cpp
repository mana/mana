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

#include "utils/xml.h"

#include "log.h"

#include "resources/resourcemanager.h"

#include <iostream>
#include <fstream>

namespace XML
{
    Document::Document(const std::string &filename, bool useResman):
        mDoc(0)
    {
        int size;
        char *data = NULL;
        if (useResman)
        {
            ResourceManager *resman = ResourceManager::getInstance();
            data = (char*) resman->loadFile(filename.c_str(), size);
        }
        else
        {
            std::ifstream file;
            file.open(filename.c_str(), std::ios::in);

            if (file.is_open())
            {
                // Get length of file
                file.seekg (0, std::ios::end);
                size = file.tellg();
                file.seekg(0, std::ios::beg);

                data = new char[size];

                file.read(data, size);
                file.close();
            }
            else
            {
                logger->log("Error loading XML file %s", filename.c_str());
            }
        }

        if (data)
        {
            mDoc = xmlParseMemory(data, size);
            free(data);

            if (!mDoc)
                logger->log("Error parsing XML file %s", filename.c_str());
        }
        else
        {
            logger->log("Error loading %s", filename.c_str());
        }
    }

    Document::Document(const char *data, int size)
    {
        mDoc = xmlParseMemory(data, size);
    }

    Document::~Document()
    {
        if (mDoc)
            xmlFreeDoc(mDoc);
    }

    xmlNodePtr Document::rootNode()
    {
        return mDoc ? xmlDocGetRootElement(mDoc) : 0;
    }

    int getProperty(xmlNodePtr node, const char* name, int def)
    {
        int &ret = def;

        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop)
        {
            ret = atoi((char*)prop);
            xmlFree(prop);
        }

        return ret;
    }

    double getFloatProperty(xmlNodePtr node, const char* name, double def)
    {
        double &ret = def;

        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop)
        {
            ret = atof((char*)prop);
            xmlFree(prop);
        }

        return ret;
    }

    std::string getProperty(xmlNodePtr node, const char *name,
                            const std::string &def)
    {
        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop)
        {
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

} // namespace XML
