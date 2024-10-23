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

#include "configuration.h"

#include "event.h"
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
    Event event(Event::ConfigOptionChanged);
    event.setString("option", key);
    event.trigger(Event::ConfigChannel);
}

std::string ConfigurationObject::getValue(const std::string &key,
                                          const std::string &deflt) const
{
    auto iter = mOptions.find(key);
    return iter != mOptions.end() ? iter->second : deflt;
}

int ConfigurationObject::getValue(const std::string &key, int deflt) const
{
    auto iter = mOptions.find(key);
    return iter != mOptions.end() ? atoi(iter->second.c_str()) : deflt;
}

unsigned ConfigurationObject::getValue(const std::string &key,
                                       unsigned deflt) const
{
    auto iter = mOptions.find(key);
    return iter != mOptions.end() ? atol(iter->second.c_str()) : deflt;
}

double ConfigurationObject::getValue(const std::string &key,
                                     double deflt) const
{
    auto iter = mOptions.find(key);
    return iter != mOptions.end() ? atof(iter->second.c_str()) : deflt;
}

void ConfigurationObject::deleteList(std::list<ConfigurationObject*> &list)
{
    for (auto element : list)
        delete element;

    list.clear();
}

void ConfigurationObject::clear()
{
    for (auto &[_, list] : mContainerOptions)
        deleteList(list);

    mOptions.clear();
}

ConfigurationObject::~ConfigurationObject()
{
    clear();
}

void Configuration::cleanDefaults()
{
    if (mDefaultsData)
    {
        for (auto &[_, variableData] : *mDefaultsData)
        {
            delete variableData;
        }
        delete mDefaultsData;
        mDefaultsData = nullptr;
    }
}

Configuration::~Configuration()
{
    cleanDefaults();
}

void Configuration::setDefaultValues(DefaultsData *defaultsData)
{
    cleanDefaults();
    mDefaultsData = defaultsData;
}

VariableData *Configuration::getDefault(const std::string &key,
                                        VariableData::DataType type) const
{
    if (mDefaultsData)
    {
        auto itdef = mDefaultsData->find(key);

        if (itdef != mDefaultsData->end() && itdef->second
            && itdef->second->getType() == type)
        {
            return itdef->second;
        }

        logger->log("%s: No value in registry for key %s",
                    mConfigPath.c_str(),
                    key.c_str());
    }

    return nullptr;
}

int Configuration::getIntValue(const std::string &key) const
{
    int defaultValue = 0;
    auto iter = mOptions.find(key);
    if (iter == mOptions.end())
    {
        VariableData *vd = getDefault(key, VariableData::DATA_INT);
        if (vd)
            defaultValue = ((IntData*)vd)->getData();
    }
    else
    {
        defaultValue = atoi(iter->second.c_str());
    }

    return defaultValue;
}

std::string Configuration::getStringValue(const std::string &key) const
{
    std::string defaultValue;
    auto iter = mOptions.find(key);
    if (iter == mOptions.end())
    {
        if (VariableData *vd = getDefault(key, VariableData::DATA_STRING))
            defaultValue = ((StringData*)vd)->getData();
    }
    else
    {
        defaultValue = iter->second;
    }

    return defaultValue;
}


float Configuration::getFloatValue(const std::string &key) const
{
    float defaultValue = 0.0f;
    auto iter = mOptions.find(key);
    if (iter == mOptions.end())
    {
        if (VariableData *vd = getDefault(key, VariableData::DATA_FLOAT))
            defaultValue = ((FloatData*)vd)->getData();
    }
    else
    {
        defaultValue = atof(iter->second.c_str());
    }

    return defaultValue;
}

bool Configuration::getBoolValue(const std::string &key) const
{
    bool defaultValue = false;
    auto iter = mOptions.find(key);
    if (iter == mOptions.end())
    {
        if (VariableData *vd = getDefault(key, VariableData::DATA_BOOL))
            defaultValue = ((BoolData*)vd)->getData();
    }
    else
    {
        return getBoolFromString(iter->second, defaultValue);
    }

    return defaultValue;
}

void ConfigurationObject::initFromXML(xmlNodePtr parent_node)
{
    clear();

    for (auto node : XML::Children(parent_node))
    {
        if (xmlStrEqual(node->name, BAD_CAST "list"))
        {
            // List option handling.
            std::string name = XML::getProperty(node, "name", std::string());

            for (auto subnode : XML::Children(node))
            {
                if (xmlStrEqual(subnode->name, BAD_CAST name.c_str())
                    && subnode->type == XML_ELEMENT_NODE)
                {
                    auto *cobj = new ConfigurationObject;

                    cobj->initFromXML(subnode); // Recurse

                    mContainerOptions[name].push_back(cobj);
                }
            }

        }
        else if (xmlStrEqual(node->name, BAD_CAST "option"))
        {
            // Single option handling.
            std::string name = XML::getProperty(node, "name", std::string());
            std::string value = XML::getProperty(node, "value", std::string());

            if (!name.empty())
                mOptions[name] = value;
        } // Otherwise ignore
    }
}

void Configuration::init(const std::string &filename, bool useResManager)
{
    XML::Document doc(filename, useResManager);

    if (useResManager)
        mConfigPath = "PhysFS://" + filename;
    else
        mConfigPath = filename;

    if (!doc.rootNode())
    {
        logger->log("Couldn't open configuration file: %s", filename.c_str());
        return;
    }

    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "configuration"))
    {
        logger->log("Warning: No configuration file (%s)", filename.c_str());
        return;
    }

    initFromXML(rootNode);
}

void ConfigurationObject::writeToXML(xmlTextWriterPtr writer)
{
    for (const auto &option : mOptions)
    {
        xmlTextWriterStartElement(writer, BAD_CAST "option");
        xmlTextWriterWriteAttribute(writer,
                BAD_CAST "name", BAD_CAST option.first.c_str());
        xmlTextWriterWriteAttribute(writer,
                BAD_CAST "value", BAD_CAST option.second.c_str());
        xmlTextWriterEndElement(writer);
    }

    for (auto &[name, list] : mContainerOptions)
    {
        xmlTextWriterStartElement(writer, BAD_CAST "list");
        xmlTextWriterWriteAttribute(writer, BAD_CAST "name", BAD_CAST name.c_str());

        // Recurse on all elements
        for (auto element : list)
        {
            xmlTextWriterStartElement(writer, BAD_CAST name.c_str());
            element->writeToXML(writer);
            xmlTextWriterEndElement(writer);
        }

        xmlTextWriterEndElement(writer);
    }
}

void Configuration::write()
{
    // Do not attempt to write to file that cannot be opened for writing
    FILE *testFile = fopen(mConfigPath.c_str(), "w");
    if (!testFile)
    {
        logger->log("Configuration::write() couldn't open %s for writing",
                    mConfigPath.c_str());
        return;
    }

    fclose(testFile);


    xmlTextWriterPtr writer = xmlNewTextWriterFilename(mConfigPath.c_str(), 0);

    if (!writer)
    {
        logger->log("Configuration::write() error while creating writer");
        return;
    }

    logger->log("Configuration::write() writing configuration...");

    xmlTextWriterSetIndent(writer, 1);
    xmlTextWriterStartDocument(writer, nullptr, nullptr, nullptr);
    xmlTextWriterStartElement(writer, BAD_CAST "configuration");

    writeToXML(writer);

    xmlTextWriterEndDocument(writer);
    xmlFreeTextWriter(writer);
}
