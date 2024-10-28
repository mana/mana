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

#ifndef XML_H
#define XML_H

#include "utils/stringutils.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>

#include <string>

/**
 * XML helper functions.
 */
namespace XML
{
    /**
     * A helper class for parsing an XML document, which also cleans it up
     * again (RAII).
     */
    class Document
    {
        public:
            /**
             * Constructor that attempts to load the given file through the
             * resource manager. Logs errors.
             */
            Document(const std::string &filename, bool useResman = true);

            /**
             * Destructor. Frees the loaded XML file.
             */
            ~Document();

            /**
             * Returns the root node of the document (or NULL if there was a
             * load error).
             */
            xmlNodePtr rootNode();

        private:
            xmlDocPtr mDoc;
    };

    /**
     * Returns whether a certain property is present.
     */
    bool hasProperty(xmlNodePtr node, const char *name);

    /**
     * Gets an integer property from an xmlNodePtr.
     */
    int getProperty(xmlNodePtr node, const char *name, int def);

    /**
     * Gets an floating point property from an xmlNodePtr.
     */
    double getFloatProperty(xmlNodePtr node, const char *name, double def);

    /**
     * Gets a string property from an xmlNodePtr.
     */
    std::string getProperty(xmlNodePtr node, const char *name,
                            const std::string &def);

    /**
     * Gets a boolean property from an xmlNodePtr.
     */
    bool getBoolProperty(xmlNodePtr node, const char *name, bool def);

    /**
     * Finds the first child node with the given name
     */
    xmlNodePtr findFirstChildByName(xmlNodePtr parent, const char *name);

    /**
     * Initializes the XML engine.
     */
    void init();

    /**
     * Helper class to iterate over the children of a node. Enables range-based
     * for loops.
     */
    class Children
    {
    public:
        class Iterator
        {
        public:
            explicit Iterator(xmlNodePtr node) : mNode(node) {}

            bool operator!=(const Iterator &other) const { return mNode != other.mNode; }
            void operator++() { mNode = mNode->next; }
            xmlNodePtr operator*() const { return mNode; }

        private:
            xmlNodePtr mNode;
        };

        explicit Children(xmlNodePtr node) : mNode(node) {}

        Iterator begin() const { return Iterator(mNode->children); }
        Iterator end() const { return Iterator(nullptr); }

    private:
        xmlNodePtr mNode;
    };

    /**
     * Helper class for writing out XML data.
     *
     * Based on libxml2's text writing API for XML.
     */
    class Writer
    {
    public:
        Writer(const std::string &fileName);
        ~Writer();

        bool isValid() const { return mWriter != nullptr; }

        void startElement(const char *name);
        void endElement();

        void addAttribute(const char *name, const std::string &value);
        void addAttribute(const char *name, const char *value);
        void addAttribute(const char *name, int value);
        void addAttribute(const char *name, unsigned value);
        void addAttribute(const char *name, float value);
        void addAttribute(const char *name, bool value);

        template<typename Enum, std::enable_if_t<std::is_enum_v<Enum>, bool> = true>
        void addAttribute(const char *name, Enum &value);

        void writeText(const std::string &text);

    private:
        xmlTextWriterPtr mWriter;
    };

    template<typename Enum, std::enable_if_t<std::is_enum_v<Enum>, bool>>
    inline void Writer::addAttribute(const char *name, Enum &value)
    {
        return addAttribute(name, static_cast<int>(value));
    }

    inline void Writer::startElement(const char *name)
    {
        xmlTextWriterStartElement(mWriter, BAD_CAST name);
    }

    inline void Writer::endElement()
    {
        xmlTextWriterEndElement(mWriter);
    }

    inline void Writer::addAttribute(const char *name, const std::string &value)
    {
        addAttribute(name, value.c_str());
    }

    inline void Writer::addAttribute(const char *name, const char *value)
    {
        xmlTextWriterWriteAttribute(mWriter, BAD_CAST name, BAD_CAST value);
    }

    inline void Writer::addAttribute(const char *name, int value)
    {
        xmlTextWriterWriteAttribute(mWriter, BAD_CAST name, BAD_CAST toString(value).c_str());
    }

    inline void Writer::addAttribute(const char *name, unsigned value)
    {
        xmlTextWriterWriteAttribute(mWriter, BAD_CAST name, BAD_CAST toString(value).c_str());
    }

    inline void Writer::addAttribute(const char *name, float value)
    {
        xmlTextWriterWriteAttribute(mWriter, BAD_CAST name, BAD_CAST toString(value).c_str());
    }

    inline void Writer::addAttribute(const char *name, bool value)
    {
        xmlTextWriterWriteAttribute(mWriter, BAD_CAST name, BAD_CAST (value ? "1" : "0"));
    }

    inline void Writer::writeText(const std::string &text)
    {
        xmlTextWriterWriteString(mWriter, BAD_CAST text.c_str());
    }

}

#endif // XML_H
