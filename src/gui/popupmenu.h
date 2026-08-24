/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "gui/widgets/menu.h"

class Being;
class FloorItem;
class Item;
class Window;

/**
 * Adds the items for changing the relation with the player \a name, which
 * depend on the current relation.
 */
void addPlayerRelationItems(Menu &menu, const std::string &name);

/**
 * The context menu shown when right-clicking a being, a floor item or an
 * item in the inventory or storage.
 */
class PopupMenu : public Menu
{
    public:
        PopupMenu() : Menu("PopupMenu") {}

        /**
         * Shows the being related popup menu at the specified mouse coords.
         */
        void showPopup(int x, int y, Being *being);

        /**
         * Shows the floor item related popup menu at the specified
         * mouse coords.
         */
        void showPopup(int x, int y, FloorItem *floorItem);

        /**
         * Shows the related popup menu when right click on the inventory
         * at the specified mouse coordinates.
         */
        void showPopup(Window *parent, int x, int y, Item *item,
                       bool isInventory, bool canDrop = true);
};
