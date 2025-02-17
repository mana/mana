/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "utils/xml.h"

#include <libxml/xmlerror.h>

#include "log.h"

#include "utils/filesystem.h"

namespace XML
{
    struct XMLContext
    {
        std::string file;
    };

#if LIBXML_VERSION >= 21200
    static void xmlLogger(void *ctx, const xmlError *error)
#else
    static void xmlLogger(void *ctx, xmlErrorPtr error)
#endif
    {
        auto *context = static_cast<XMLContext*>(ctx);

        if (context)
            logger->log("Error in XML file '%s' on line %d",
                        context->file.c_str(), error->line);
        else
            logger->log("Error in unknown XML file on line %d",
                        error->line);

        logger->log("%s", error->message);

        // No need to keep errors around
        xmlCtxtResetLastError(error->ctxt);
    }

    Document::Document(const std::string &filename, bool useResman):
        mDoc(nullptr)
    {
        XMLContext ctx;
        ctx.file = filename;
        xmlSetStructuredErrorFunc(&ctx, xmlLogger);

        size_t size;
        char *data = nullptr;

        if (useResman)
            data = (char *) FS::loadFile(filename, size);
        else
            data = (char *) SDL_LoadFile(filename.c_str(), &size);

        if (data)
        {
            mDoc = xmlParseMemory(data, size);
            SDL_free(data);

            if (!mDoc)
                logger->log("Error parsing XML file %s", filename.c_str());
        }
        else
        {
            logger->log("Error loading %s: %s", filename.c_str(), SDL_GetError());
        }

        xmlSetStructuredErrorFunc(nullptr, xmlLogger);
    }

    Document::~Document()
    {
        if (mDoc)
            xmlFreeDoc(mDoc);
    }

    void init()
    {
        // Initialize libxml2 and check for potential ABI mismatches between
        // compiled version and the shared library actually used.
        xmlInitParser();
        LIBXML_TEST_VERSION;

        // Handle libxml2 error messages
        xmlSetStructuredErrorFunc(nullptr, xmlLogger);
    }


    Writer::Writer(const std::string &fileName)
    {
        mWriter = xmlNewTextWriterFilename(fileName.c_str(), 0);
        if (!mWriter)
        {
            logger->log("Error creating XML writer for file %s", fileName.c_str());
            return;
        }

        xmlTextWriterSetIndent(mWriter, 1);
        xmlTextWriterStartDocument(mWriter, nullptr, nullptr, nullptr);
    }

    Writer::~Writer()
    {
        if (!mWriter)
            return;

        xmlTextWriterEndDocument(mWriter);
        xmlFreeTextWriter(mWriter);
    }

} // namespace XML
