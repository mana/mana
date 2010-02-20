/*
 *  Support for custom units
 *  Copyright (C) 2009-2010  The Mana World Development Team
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

#ifndef UNITS_H
#define UNITS_H

#include <string>

class Units
{
    public:
        /**
        * Loads and parses the units.xml file (if found).
        */
        static void loadUnits();

        /**
        * Formats the given number in the correct currency format.
        */
        static std::string formatCurrency(int value);

        /**
        * Formats the given number in the correct weight/mass format.
        */
        static std::string formatWeight(int value);
};

#endif // UNITS_H
