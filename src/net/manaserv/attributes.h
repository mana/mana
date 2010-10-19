/*
 *  The Mana Client
 *  Copyright (C) 2010  The Mana Developers
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

#ifndef NET_MANASERV_ATTRIBUTES_H
#define NET_MANASERV_ATTRIBUTES_H

#include <string>
#include <vector>

namespace ManaServ {
namespace Attributes {

    void load();

    void unload();

    void informItemDB();

    void informStatusWindow();

    /**
     * Returns the list of base attribute labels.
     */
    std::vector<std::string>& getLabels();

    /**
     * Give back the corresponding playerinfo Id from the attribute id
     * defined in the xml file.
     */
    int getPlayerInfoIdFromAttrId(int attrId);

    /**
     * Give the attribute points given to a character
     * at its creation.
     */
    unsigned int getCreationPoints();

    /**
     * Give the minimum attribute point possible
     * at character's creation.
     */
    unsigned int getAttributeMinimum();

    /**
     * Give the maximum attribute point possible
     * at character's creation.
     */
    unsigned int getAttributeMaximum();

} // namespace Attributes
} // namespace ManaServ

#endif // NET_MANASERV_ATTRIBUTES_H
