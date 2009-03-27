/*
 *  Color database
 *  Copyright (C) 2008  Aethyra Development Team
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

#ifndef COLOR_MANAGER_H
#define COLOR_MANAGER_H

#include <map>
#include <string>

/**
 * Color information database.
 */
namespace ColorDB
{
    /**
     * Loads the color data from <code>colors.xml</code>.
     */
    void load();

    /**
     * Clear the color data
     */
    void unload();

    std::string &get(int id);

    int size();

    // Color DB
    typedef std::map<int, std::string> Colors;
    typedef Colors::iterator ColorIterator;
}

#endif
