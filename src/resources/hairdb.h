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

#ifndef HAIR_MANAGER_H
#define HAIR_MANAGER_H

#include "iteminfo.h"

#include <map>
#include <string>
#include "utils/xml.h"

/**
 * Hair information database.
 */
class HairDB
{
  public:
    HairDB():
        mLoaded(false)
    {}

    ~HairDB()
    { unload(); }

    void init();

    void readHairColorNode(xmlNodePtr node, const std::string &filename);

    void checkStatus();

    /**
     * Clear the color data
     */
    void unload();

    const std::string &getHairColor(int id);

    /**
     * Returns the available hair style ids
     * @param maxId the max permited id. If not 0, the hairDb won't
     * return ids > to the parameter.
     */
    std::vector<int> getHairStyleIds(int maxId = 0) const;

    /**
     * Returns the available hair color ids
     * @param maxId the max permited id. If not 0, the hairDb won't
     * return ids > to the parameter.
     */
    std::vector<int> getHairColorIds(int maxId = 0) const;

    /**
     * Add a hair style to the database.
     * @see ItemDB for the itemInfo.
     */
    void addHairStyle(int id);

  private:
    /**
     * Load the hair colors, contained in a <colors> node.
     */
    void loadHairColorsNode(xmlNodePtr colorsNode);

    /**
     * Load the hair styles, contained in a <styles> node.
     * Used only by Manaserv. TMW-Athena is considering hairstyles as items.
     * @see ItemDB
     */
    void loadHairStylesNode(xmlNodePtr stylesNode);

    // Hair colors Db
    typedef std::map<int, std::string> Colors;
    typedef Colors::iterator ColorIterator;
    typedef Colors::const_iterator ColorConstIterator;
    Colors mHairColors;

    typedef std::set<int> HairStyles;
    typedef HairStyles::iterator HairStylesIterator;
    typedef HairStyles::const_iterator HairStylesConstIterator;
    HairStyles mHairStyles;

    bool mLoaded;
};

extern HairDB hairDB;

#endif
