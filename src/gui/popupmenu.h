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

#ifndef POPUP_MENU_H
#define POPUP_MENU_H

#include "gui/linkhandler.h"
#include "gui/widgets/popup.h"

class Being;
class BrowserBox;
class FloorItem;
class Item;

/**
 * Window showing popup menu.
 */
class PopupMenu : public Popup, public LinkHandler
{
    public:
        /**
         * Constructor.
         */
        PopupMenu();

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
        void showPopup(int x, int y, Item *item, bool isInventory);

        /**
         * Handles link action.
         */
        void handleLink(const std::string &link);

    private:
        BrowserBox* mBrowserBox;

        int mBeingId;
        FloorItem* mFloorItem;
        Item *mItem;

        /**
         * Shared code for the various showPopup functions.
         */
        void showPopup(int x, int y);
};

#endif
