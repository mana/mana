/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#ifndef CONFIGLISTENER_H
#define CONFIGLISTENER_H

#include <string>

/**
 * The listener interface for receiving notifications about changes to
 * configuration options.
 *
 * \ingroup CORE
 */
class ConfigListener
{
    public:
        /**
         * Destructor.
         */
        virtual ~ConfigListener() {};

        /**
         * Called when an option changed. The config listener will have to be
         * registered to the option name first.
         */
        virtual void optionChanged(const std::string &name) = 0;
};

#endif
