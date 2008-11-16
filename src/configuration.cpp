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


#include "configuration.h"

#include <libxml/xmlwriter.h>

#include "configlistener.h"
#include "log.h"

#include "utils/tostring.h"
#include "utils/xml.h"

void Configuration::init(const std::string &filename)
{
    mConfigPath = filename;

    // Do not attempt to read config from non-existant file
    FILE *testFile = fopen(filename.c_str(), "r");
    if (!testFile) {
        return;
    }
    else {
        fclose(testFile);
    }

    xmlDocPtr doc = xmlReadFile(filename.c_str(), NULL, 0);

    if (!doc) return;

    xmlNodePtr rootNode = xmlDocGetRootElement(doc);

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "configuration")) {
        logger->log("Warning: No configuration file (%s)", filename.c_str());
        xmlFreeDoc(doc);
        return;
    }

    for_each_xml_child_node(node, rootNode)
    {
        if (!xmlStrEqual(node->name, BAD_CAST "option"))
            continue;

        std::string name = XML::getProperty(node, "name", std::string());
        std::string value = XML::getProperty(node, "value", std::string());

        if (!name.empty() && !value.empty())
        {
            mOptions[name] = value;
        }
    }

    xmlFreeDoc(doc);
}

void Configuration::write()
{
    // Do not attempt to write to file that cannot be opened for writing
    FILE *testFile = fopen(mConfigPath.c_str(), "w");
    if (!testFile) {
        logger->log("Configuration::write() couldn't open %s for writing",
                    mConfigPath.c_str());
        return;
    }
    else {
        fclose(testFile);
    }

    xmlTextWriterPtr writer = xmlNewTextWriterFilename(mConfigPath.c_str(), 0);

    if (!writer) {
        logger->log("Configuration::write() error while creating writer");
        return;
    }

    logger->log("Configuration::write() writing configuration...");

    xmlTextWriterSetIndent(writer, 1);
    xmlTextWriterStartDocument(writer, NULL, NULL, NULL);
    xmlTextWriterStartElement(writer, BAD_CAST "configuration");

    for (OptionIterator i = mOptions.begin(); i != mOptions.end(); i++)
    {
        xmlTextWriterStartElement(writer, BAD_CAST "option");
        xmlTextWriterWriteAttribute(writer,
                BAD_CAST "name", BAD_CAST i->first.c_str());
        xmlTextWriterWriteAttribute(writer,
                BAD_CAST "value", BAD_CAST i->second.c_str());
        xmlTextWriterEndElement(writer);
    }

    xmlTextWriterEndDocument(writer);
    xmlFreeTextWriter(writer);
}

void Configuration::setValue(const std::string &key, std::string value)
{
    mOptions[key] = value;

    // Notify listeners
    ListenerMapIterator list = mListenerMap.find(key);
    if (list != mListenerMap.end()) {
        Listeners listeners = list->second;
        for (ListenerIterator i = listeners.begin(); i != listeners.end(); i++)
        {
            (*i)->optionChanged(key);
        }
    }
}

void Configuration::setValue(const std::string &key, float value)
{
    setValue(key, toString((value == (int)value) ? (int)value : value));
}

std::string Configuration::getValue(const std::string &key, std::string deflt)
{
    OptionIterator iter = mOptions.find(key);
    return ((iter != mOptions.end()) ? iter->second : deflt);
}

float Configuration::getValue(const std::string &key, float deflt)
{
    OptionIterator iter = mOptions.find(key);
    return (iter != mOptions.end()) ? atof(iter->second.c_str()) : deflt;
}

void Configuration::addListener(
        const std::string &key, ConfigListener *listener)
{
    mListenerMap[key].push_front(listener);
}

void Configuration::removeListener(
        const std::string &key, ConfigListener *listener)
{
    mListenerMap[key].remove(listener);
}
