/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id: popupmenu.h 2239 2006-03-09 05:16:27Z der_doener $
 */

#ifndef _TMW_POPUP_MENU_H
#define _TMW_POPUP_MENU_H

#include "window.h"
#include "linkhandler.h"

class Being;
class BrowserBox;
class FloorItem;
class Item;


/**
 * Window showing popup menu.
 */
class PopupMenu : public Window, public LinkHandler
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
        void showPopup(int x, int y, Item *item);

        /**
         * Handles link action.
         */
        void handleLink(const std::string& link);

    private:
        BrowserBox* mBrowserBox;

        Being* mBeing;
        FloorItem* mFloorItem;
        Item *mItem;

        /**
         * Shared code for the various showPopup functions.
         */
        void showPopup(int x, int y);
};

#endif
