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

#include <iostream>
#include <fstream>
#include <cstring>

#include <libxml/parser.h>
#include <libxml/xmlerror.h>

#include "log.h"

#include "resources/resourcemanager.h"

#include "utils/stringutils.h"
#include "utils/zlib.h"

namespace XML
{
    static void xmlLogger(void *ctx, xmlErrorPtr error);

    struct XMLContext
    {
        std::string file;
        bool resman;
    };

    Document::Document(const std::string &filename, bool useResman):
        mDoc(0)
    {
        XMLContext ctx;
        ctx.file = filename;
        ctx.resman = useResman;
        xmlSetStructuredErrorFunc(&ctx, xmlLogger);

        int size;
        char *data = NULL;
        if (useResman)
        {
            ResourceManager *resman = ResourceManager::getInstance();
            data = (char*) resman->loadFile(filename.c_str(), size);
        }
        else
        {
            data = (char *) loadCompressedFile(filename, size);
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

        xmlSetStructuredErrorFunc(NULL, xmlLogger);
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

    bool getBoolProperty(xmlNodePtr node, const char* name, bool def)
    {
        bool ret = def;
        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop)
        {
            ret = getBoolFromString((char*) prop, def);
            xmlFree(prop);
        }
        return ret;
    }

    xmlNodePtr findFirstChildByName(xmlNodePtr parent, const char *name)
    {
        for_each_xml_child_node(child, parent)
            if (xmlStrEqual(child->name, BAD_CAST name))
                return child;

        return NULL;
    }

    static void xmlLogger(void *ctx, xmlErrorPtr error)
    {
        XMLContext *context = static_cast<XMLContext*>(ctx);

        if (context)
            logger->log("Error in XML file '%s' on line %d",
                        context->file.c_str(), error->line);
        else
            logger->log("Error in unknown xml file on line %d",
                        error->line);

        logger->log("%s", error->message);

        // No need to keep errors around
        xmlCtxtResetLastError(error->ctxt);
    }

    void init()
    {
        // Initialize libxml2 and check for potential ABI mismatches between
        // compiled version and the shared library actually used.
        xmlInitParser();
        LIBXML_TEST_VERSION;

        // Handle libxml2 error messages
        xmlSetStructuredErrorFunc(NULL, xmlLogger);
    }

} // namespace XML
