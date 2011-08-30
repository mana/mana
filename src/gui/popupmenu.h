/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef POPUP_MENU_H
#define POPUP_MENU_H

#include "gui/widgets/linkhandler.h"
#include "gui/widgets/popup.h"

class Being;
class BrowserBox;
class FloorItem;
class Item;
class Window;

/**
 * Window showing popup menu.
 */
class PopupMenu : public Popup, public LinkHandler
{
    public:
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
        void showPopup(Window *parent, int x, int y, Item *item,
                       bool isInventory, bool canDrop = true);

        /**
         * Handles link action.
         */
        void handleLink(const std::string &link);

    private:
        BrowserBox* mBrowserBox;

        int mBeingId;
        FloorItem* mFloorItem;
        Item *mItem;

        Window *mWindow;

        /**
         * Shared code for the various showPopup functions.
         */
        void showPopup(int x, int y);
};

#endif
