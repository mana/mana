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

#ifndef ITEM_MANAGER_H
#define ITEM_MANAGER_H

#include <list>
#include <map>
#include <string>

class ItemInfo;

/**
 * Item information database.
 */
namespace ItemDB
{
    /**
     * Loads the item data from <code>items.xml</code>.
     */
    void load();

    /**
     * Frees item data.
     */
    void unload();

    const ItemInfo &get(int id);
    const ItemInfo &get(const std::string &name);

    struct Stat
    {
        Stat(const std::string &tag,
             const std::string &format):
            tag(tag),
            format(format)
        {}

        std::string tag;
        std::string format;
    };

    void setStatsList(const std::list<Stat> &stats);

    // Items database
    typedef std::map<int, ItemInfo*> ItemInfos;
    typedef std::map<std::string, ItemInfo*> NamedItemInfos;
}

#endif
