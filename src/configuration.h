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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "utils/stringutils.h"

#include <libxml/xmlwriter.h>

#include <cassert>
#include <list>
#include <map>
#include <string>

class ConfigListener;
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
         * \param IT Iterator type over CONT
         * \param T Elements that IT iterates over
         * \param CONT The associated container type
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
            ConfigurationObject *nextobj = new ConfigurationObject;
            deleteList(name);
            ConfigurationList *list = &(mContainerOptions[name]);

            for (IT it = begin; it != end; it++) {
                ConfigurationObject *wrobj = manager->writeConfigItem(*it, nextobj);
                if (wrobj) { // wrote something
                    assert (wrobj == nextobj);
                    nextobj = new ConfigurationObject;
                    list->push_back(wrobj);
                } else
                    nextobj->clear(); // you never know...
            }

            delete nextobj;
        }

       /**
        * Serialises a container into a list of configuration options
        *
        * \param IT Iterator type over CONT
        * \param T Elements that IT iterates over
        * \param CONT The associated container type
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

        typedef std::map<std::string, std::string> Options;
        Options mOptions;

        typedef std::list<ConfigurationObject *> ConfigurationList;
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
        virtual ~Configuration() {}

        /**
         * Reads config file and parse all options into memory.
         *
         * \param filename path to config file
         */
        void init(const std::string &filename);

        /**
         * Writes the current settings back to the config file.
         */
        void write();

        /**
         * Adds a listener to the listen list of the specified config option.
         */
        void addListener(const std::string &key, ConfigListener *listener);

        /**
         * Removes a listener from the listen list of the specified config
         * option.
         */
        void removeListener(const std::string &key, ConfigListener *listener);

        void setValue(const std::string &key, const std::string &value);

        inline void setValue(const std::string &key, float value)
        { setValue(key, toString(value)); }

        inline void setValue(const std::string &key, double value)
        { setValue(key, toString(value)); }

        inline void setValue(const std::string &key, int value)
        { setValue(key, toString(value)); }

        inline void setValue(const std::string &key, unsigned value)
        { setValue(key, toString(value)); }

        inline void setValue(const std::string &key, bool value)
        { setValue(key, value ? std::string("1") : std::string("0")); }

    private:
        typedef std::list<ConfigListener*> Listeners;
        typedef Listeners::iterator ListenerIterator;
        typedef std::map<std::string, Listeners> ListenerMap;
        typedef ListenerMap::iterator ListenerMapIterator;
        ListenerMap mListenerMap;

        std::string mConfigPath;         /**< Location of config file */
};

extern Configuration branding;
extern Configuration config;

#endif
