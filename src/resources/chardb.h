/*
 *  Character creation settings
 *  Copyright (C) 2011-2013  The ManaPlus Developers
 *  Copyright (C) 2013  The Mana Developers
 *
 *  This file is part of The ManaPlus Client.
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

#ifndef RESOURCES_CHARDB_H
#define RESOURCES_CHARDB_H

#include <vector>

/**
 * Character creation settings.
 */
namespace CharDB
{
    void load();
    void unload();

    unsigned getMinHairColor();
    unsigned getMaxHairColor();

    unsigned getMinHairStyle();
    unsigned getMaxHairStyle();

    unsigned getMinStat();
    unsigned getMaxStat();
    unsigned getSumStat();

    const std::vector<int> &getDefaultItems();
}

#endif // RESOURCES_CHARDB_H
