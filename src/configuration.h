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

#ifndef _TMW_CONFIGURATION_H
#define _TMW_CONFIGURATION_H

#include <map>
#include <list>
#include <string>

class ConfigListener;

/**
 * Configuration handler for reading (and writing).
 *
 * \ingroup CORE
 */
class Configuration
{
    public:
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
         * Sets an option using a string value.
         *
         * \param key Option identifier.
         * \param value Value.
         */
        void setValue(const std::string &key, std::string value);

        /**
         * Sets an option using a numeric value.
         *
         * \param key Option identifier.
         * \param value Value.
         */
        void setValue(const std::string &key, float value);

        /**
         * Gets a value as string.
         *
         * \param key Option identifier.
         * \param deflt Default option if not there or error.
         */
        std::string getValue(const std::string &key, std::string deflt);

        /**
         * Gets a value as numeric (float).
         *
         * \param key Option identifier.
         * \param deflt Default option if not there or error.
         */
        float getValue(const std::string &key, float deflt);

        /**
         * Adds a listener to the listen list of the specified config option.
         */
        void addListener(const std::string &key, ConfigListener *listener);

        /**
         * Removes a listener from the listen list of the specified config
         * option.
         */
        void removeListener(const std::string &key, ConfigListener *listener);

    private:
        typedef std::map<std::string, std::string> Options;
        typedef Options::iterator OptionIterator;
        Options mOptions;

        typedef std::list<ConfigListener*> Listeners;
        typedef Listeners::iterator ListenerIterator;
        typedef std::map<std::string, Listeners> ListenerMap;
        typedef ListenerMap::iterator ListenerMapIterator;
        ListenerMap mListenerMap;

        std::string mConfigPath;         /**< Location of config file */
};

extern Configuration config;

#endif
