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

#pragma once

#include "utils/stringutils.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>

#include <string_view>

/**
 * XML helper functions.
 */
namespace XML
{
    class Node
    {
    public:
        Node(xmlNodePtr node = nullptr)
            : node(node)
        {}

        operator bool() const { return node != nullptr; }

        std::string_view name() const { return (const char *) node->name; }
        std::string_view textContent() const;

        template<typename T>
        bool attribute(const char *name, T &value);

        bool hasAttribute(const char *name) const;
        int getProperty(const char *name, int def) const;
        double getFloatProperty(const char *name, double def) const;
        std::string getProperty(const char *name, const std::string &def) const;
        bool getBoolProperty(const char *name, bool def) const;
        Node findFirstChildByName(const char *name) const;

        /**
         * Helper class to iterate over the children of a node. Enables
         * range-based for loops.
         */
        class Children
        {
        public:
            class Iterator
            {
            public:
                explicit Iterator(xmlNodePtr node) : mNode(node)
                {
                    while (mNode && mNode->type != XML_ELEMENT_NODE)
                        mNode = mNode->next;
                }

                bool operator!=(const Iterator &other) const { return mNode != other.mNode; }
                void operator++()
                {
                    do {
                        mNode = mNode->next;
                    } while (mNode && mNode->type != XML_ELEMENT_NODE);
                }
                Node operator*() const { return mNode; }

            private:
                xmlNodePtr mNode;
            };

            explicit Children(xmlNodePtr node) : mNode(node) {}

            Iterator begin() const { return Iterator(mNode->children); }
            Iterator end() const { return Iterator(nullptr); }

        private:
            xmlNodePtr mNode;
        };

        Children children() const { return Children(node); }

    private:
        const char *attribute(const char *name) const;

        xmlNodePtr node;
    };

    inline std::string_view Node::textContent() const
    {
        if (node->children && node->children->type == XML_TEXT_NODE)
            return (const char *) node->children->content;
        return {};
    }

    inline const char *Node::attribute(const char *name) const
    {
        if (node->type != XML_ELEMENT_NODE)
            return nullptr;

        for (xmlAttrPtr prop = node->properties; prop; prop = prop->next) {
            if (xmlStrEqual(prop->name, BAD_CAST name)) {
                if (prop->children)
                    return reinterpret_cast<const char*>(prop->children->content);
                else
                    return nullptr;
            }
        }
        return nullptr;
    }

    template<typename T>
    inline bool Node::attribute(const char *name, T &value)
    {
        if (const char *str = attribute(name))
        {
            fromString(str, value);
            return true;
        }
        return false;
    }

    inline bool Node::hasAttribute(const char *name) const
    {
        return attribute(name) != nullptr;
    }

    inline int Node::getProperty(const char *name, int def) const
    {
        int ret = def;
        if (const char *str = attribute(name))
            fromString(str, ret);
        return ret;
    }

    inline double Node::getFloatProperty(const char *name, double def) const
    {
        double ret = def;
        if (const char *str = attribute(name))
            fromString(str, ret);
        return ret;
    }

    inline std::string Node::getProperty(const char *name, const std::string &def) const
    {
        if (const char *str = attribute(name))
            return str;

        return def;
    }

    inline bool Node::getBoolProperty(const char *name, bool def) const
    {
        bool ret = def;
        if (const char *str = attribute(name))
            ret = getBoolFromString(str, def);
        return ret;
    }

    inline Node Node::findFirstChildByName(const char *name) const
    {
        for (auto child : children())
            if (child.name() == name)
                return child;

        return nullptr;
    }

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
        Node rootNode();

    private:
        xmlDocPtr mDoc;
    };

    inline Node Document::rootNode()
    {
        return mDoc ? xmlDocGetRootElement(mDoc) : nullptr;
    }

    /**
     * Initializes the XML engine.
     */
    void init();

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
