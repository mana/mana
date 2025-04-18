/*
 *  Hair database
 *  Copyright (C) 2008  Aethyra Development Team
 *  Copyright (C) 2009-2013  The Mana Developers
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

#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>
#include "utils/xml.h"

/**
 * Hair information database.
 */
class HairDB
{
public:
    HairDB() = default;

    ~HairDB()
    { unload(); }

    void init();

    void readHairColorNode(XML::Node node, const std::string &filename);

    void checkStatus();

    /**
     * Clear the color data
     */
    void unload();

    const std::string &getHairColor(int id) const;

    /**
     * Returns the available hair style ids
     * @param maxId the max permited id. If not 0, the hairDb won't
     * return ids > to the parameter.
     */
    std::vector<int> getHairStyleIds(int maxId) const;

    /**
     * Returns the available hair color ids
     * @param maxId the max permited id. If not 0, the hairDb won't
     * return ids > to the parameter.
     */
    std::vector<int> getHairColorIds(int minId, int maxId) const;

    /**
     * Add a hair style to the database.
     * @see ItemDB for the itemInfo.
     */
    void addHairStyle(int id);

private:
    /**
     * Load the hair colors, contained in a <colors> node.
     */
    void loadHairColorsNode(XML::Node colorsNode);

    /**
     * Load the hair styles, contained in a <styles> node.
     * Used only by Manaserv. TMW-Athena is considering hairstyles as items.
     * @see ItemDB
     */
    void loadHairStylesNode(XML::Node stylesNode);

    // Hair colors Db
    std::map<int, std::string> mHairColors;
    std::set<int> mHairStyles;

    bool mLoaded = false;
};

extern HairDB hairDB;
