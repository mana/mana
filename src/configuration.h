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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "utils/stringutils.h"
#include "defaults.h"

#include <libxml/xmlwriter.h>

#include <cassert>
#include <list>
#include <map>
#include <string>

class ConfigurationObject;

/**
 * Configuration list manager interface; responsible for
 * serializing/deserializing configuration choices in containers.
 *
 * \param T Type of the container elements to serialise
 * \param CONT Type of the container we (de)serialise
 */
template <class T, class CONT>
class ConfigurationListManager
{
    public:
        /**
         * Writes a value into a configuration object
         *
         * \param value The value to write out
         * \param obj The configuation object to write to
         * \return obj, or otherwise NULL to indicate that this option should
         *         be skipped
         */
        virtual ConfigurationObject *writeConfigItem(T value,
                                                     ConfigurationObject *obj) = 0;

        /**
         * Reads a value from a configuration object
         *
         * \param obj The configuration object to read from
         * \param container The container to insert the object to
         */
        virtual CONT readConfigItem(ConfigurationObject *obj,
                                    CONT container) = 0;

        virtual ~ConfigurationListManager() {}
};

/**
 * Configuration object, mapping values to names and possibly containing
 * lists of further configuration objects
 *
 * \ingroup CORE
 */
class ConfigurationObject
{
    friend class Configuration;

    public:
        virtual ~ConfigurationObject();

        /**
         * Sets an option using a string value.
         *
         * \param key Option identifier.
         * \param value Value.
         */
        virtual void setValue(const std::string &key,
                              const std::string &value);

        /**
         * Gets a value as string.
         *
         * \param key Option identifier.
         * \param deflt Default option if not there or error.
         */
        std::string getValue(const std::string &key,
                             const std::string &deflt) const;

        int getValue(const std::string &key, int deflt) const;

        unsigned getValue(const std::string &key, unsigned deflt) const;

        double getValue(const std::string &key, double deflt) const;

        /**
         * Re-sets all data in the configuration
         */
        virtual void clear();

        /**
         * Serialises a container into a list of configuration options
         *
         * \tparam IT Iterator type over CONT
         * \tparam T Elements that IT iterates over
         * \tparam CONT The associated container type
         *
         * \param name Name of the list the elements should be stored under
         * \param begin Iterator start
         * \param end Iterator end
         * \param manager An object capable of serialising T items
         */
        template <class IT, class T, class CONT>
        void setList(const std::string &name, IT begin, IT end,
                     ConfigurationListManager<T, CONT> *manager)
        {
            auto *nextobj = new ConfigurationObject;
            deleteList(name);
            ConfigurationList *list = &(mContainerOptions[name]);

            for (IT it = begin; it != end; it++)
            {
                ConfigurationObject *wrobj = manager->writeConfigItem(*it, nextobj);
                if (wrobj)
                { // wrote something
                    assert (wrobj == nextobj);
                    nextobj = new ConfigurationObject;
                    list->push_back(wrobj);
                }
                else
                {
                    nextobj->clear(); // you never know...
                }
            }

            delete nextobj;
        }

       /**
        * Serialises a container into a list of configuration options
        *
        * \tparam IT Iterator type over CONT
        * \tparam T Elements that IT iterates over
        * \tparam CONT The associated container type
        *
        * \param name Name of the list the elements should be read from under
        * \param empty Initial (empty) container to write to
        * \param manager An object capable of deserialising items into CONT
        */
        template<class T, class CONT>
        CONT getList(const std::string &name, CONT empty, ConfigurationListManager<T, CONT> *manager)
        {
            ConfigurationList *list = &(mContainerOptions[name]);
            CONT container = empty;

            for (ConfigurationList::const_iterator it = list->begin(); it != list->end(); it++)
                container = manager->readConfigItem(*it, container);

            return container;
        }

    protected:
        virtual void initFromXML(xmlNodePtr node);
        virtual void writeToXML(xmlTextWriterPtr writer);

        void deleteList(const std::string &name);

        using Options = std::map<std::string, std::string>;
        Options mOptions;

        using ConfigurationList = std::list<ConfigurationObject *>;
        std::map<std::string, ConfigurationList> mContainerOptions;
};

/**
 * Configuration handler for reading (and writing).
 *
 * \ingroup CORE
 */
class Configuration : public ConfigurationObject
{
    public:
        Configuration() = default;

        ~Configuration() override;

        /**
         * Reads config file and parse all options into memory.
         *
         * @param filename path to config file
         * @param useResManager Make use of the resource manager.
         */
        void init(const std::string &filename, bool useResManager = false);

        /**
         * Set the default values for each keys.
         *
         * @param defaultsData data used as defaults.
         */
        void setDefaultValues(DefaultsData *defaultsData);

        /**
         * Writes the current settings back to the config file.
         */
        void write();

        void setValue(const std::string &key, const std::string &value) override;

        void setValue(const std::string &key, const char *value)
        { setValue(key, std::string(value)); }

        void setValue(const std::string &key, float value)
        { setValue(key, toString(value)); }

        void setValue(const std::string &key, double value)
        { setValue(key, toString(value)); }

        void setValue(const std::string &key, int value)
        { setValue(key, toString(value)); }

        void setValue(const std::string &key, unsigned value)
        { setValue(key, toString(value)); }

        void setValue(const std::string &key, bool value)
        { setValue(key, value ? "1" : "0"); }

        /**
         * returns a value corresponding to the given key.
         * The default value returned in based on fallbacks registry.
         * @see defaults.h
         */
        int getIntValue(const std::string &key) const;

        float getFloatValue(const std::string &key) const;

        std::string getStringValue(const std::string &key) const;

        bool getBoolValue(const std::string &key) const;

        VariableData* getDefault(const std::string &key,
                                       VariableData::DataType type) const;
    private:
        /**
         * Clean up the default values member.
         */
        void cleanDefaults();

        std::string mConfigPath;       /**< Location of config file */
        DefaultsData *mDefaultsData = nullptr;   /**< Defaults of value for a given key */
};

extern Configuration branding;
extern Configuration config;
extern Configuration paths;

#endif
