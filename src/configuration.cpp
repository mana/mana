/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "configlistener.h"
#include "configuration.h"
#include "log.h"

#include "utils/stringutils.h"
#include "utils/xml.h"

void ConfigurationObject::setValue(const std::string &key,
                                   const std::string &value)
{
    mOptions[key] = value;
}

void Configuration::setValue(const std::string &key, const std::string &value)
{
    ConfigurationObject::setValue(key, value);

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

std::string ConfigurationObject::getValue(const std::string &key,
                                          const std::string &deflt) const
{
    Options::const_iterator iter = mOptions.find(key);
    return ((iter != mOptions.end()) ? iter->second : deflt);
}

int ConfigurationObject::getValue(const std::string &key, int deflt) const
{
    Options::const_iterator iter = mOptions.find(key);
    return (iter != mOptions.end()) ? atoi(iter->second.c_str()) : deflt;
}

unsigned ConfigurationObject::getValue(const std::string &key,
                                       unsigned deflt) const
{
    Options::const_iterator iter = mOptions.find(key);
    return (iter != mOptions.end()) ? atol(iter->second.c_str()) : deflt;
}

double ConfigurationObject::getValue(const std::string &key,
                                     double deflt) const
{
    Options::const_iterator iter = mOptions.find(key);
    return (iter != mOptions.end()) ? atof(iter->second.c_str()) : deflt;
}

void ConfigurationObject::deleteList(const std::string &name)
{
    for (ConfigurationList::const_iterator
             it = mContainerOptions[name].begin(); it != mContainerOptions[name].end(); it++)
        delete *it;

    mContainerOptions[name].clear();
}

void ConfigurationObject::clear()
{
    for (std::map<std::string, ConfigurationList>::const_iterator
             it = mContainerOptions.begin(); it != mContainerOptions.end(); it++)
        deleteList(it->first);
    mOptions.clear();
}

ConfigurationObject::~ConfigurationObject()
{
    clear();
}

void ConfigurationObject::initFromXML(xmlNodePtr parent_node)
{
    clear();

    for_each_xml_child_node(node, parent_node)
    {
        if (xmlStrEqual(node->name, BAD_CAST "list")) {
            // list option handling

            std::string name = XML::getProperty(node, "name", std::string());

            for_each_xml_child_node(subnode, node)
            {
                if (xmlStrEqual(subnode->name, BAD_CAST name.c_str())
                    && subnode->type == XML_ELEMENT_NODE) {
                    ConfigurationObject *cobj = new ConfigurationObject;

                    cobj->initFromXML(subnode); // recurse

                    mContainerOptions[name].push_back(cobj);
                }
            }

        } else if (xmlStrEqual(node->name, BAD_CAST "option")) {
            // single option handling

            std::string name = XML::getProperty(node, "name", std::string());
            std::string value = XML::getProperty(node, "value", std::string());

            if (!name.empty() && !value.empty())
                mOptions[name] = value;
        } // otherwise ignore
    }
}

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

    initFromXML(rootNode);

    xmlFreeDoc(doc);
}

void ConfigurationObject::writeToXML(xmlTextWriterPtr writer)
{
    for (Options::const_iterator i = mOptions.begin(), i_end = mOptions.end();
         i != i_end; ++i)
    {
        xmlTextWriterStartElement(writer, BAD_CAST "option");
        xmlTextWriterWriteAttribute(writer,
                BAD_CAST "name", BAD_CAST i->first.c_str());
        xmlTextWriterWriteAttribute(writer,
                BAD_CAST "value", BAD_CAST i->second.c_str());
        xmlTextWriterEndElement(writer);
    }

    for (std::map<std::string, ConfigurationList>::const_iterator
             it = mContainerOptions.begin(); it != mContainerOptions.end(); it++) {
        const char *name = it->first.c_str();

        xmlTextWriterStartElement(writer, BAD_CAST "list");
        xmlTextWriterWriteAttribute(writer, BAD_CAST "name", BAD_CAST name);

        // recurse on all elements
        for (ConfigurationList::const_iterator
                 elt_it = it->second.begin(); elt_it != it->second.end(); elt_it++) {

            xmlTextWriterStartElement(writer, BAD_CAST name);
            (*elt_it)->writeToXML(writer);
            xmlTextWriterEndElement(writer);
        }

        xmlTextWriterEndElement(writer);
    }
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

    writeToXML(writer);

    xmlTextWriterEndDocument(writer);
    xmlFreeTextWriter(writer);
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
