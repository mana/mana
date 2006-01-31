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
 *
 *  $Id$
 */

#ifndef _TMW_PROPERTIES_H_
#define _TMW_PROPERTIES_H_

#include <map>
#include <string>

/**
 * A class holding a set of properties.
 */
class Properties
{
    public:
        virtual
        ~Properties() {}

        /**
         * Get a map property.
         *
         * @return the value of the given property or an empty string when it
         *         doesn't exist.
         */
        const std::string&
        getProperty(const std::string &name)
        {
            const static std::string undefined = "";
            std::map<std::string, std::string>::const_iterator i =
                properties.find(name);

            if (i != properties.end())
            {
                return (*i).second;
            }
            else
            {
                return undefined;
            }
        }

        /**
         * Returns whether a certain property is available.
         */
        bool
        hasProperty(const std::string &name)
        {
            return (properties.find(name) != properties.end());
        }

        /**
         * Set a map property.
         */
        void
        setProperty(const std::string &name, const std::string &value)
        {
            properties[name] = value;
        }

    protected:
        std::map<std::string, std::string> properties;
};

#endif
