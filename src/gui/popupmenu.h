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
 *  $Id$
 */

#ifndef _TMW_POPUP_MENU_H
#define _TMW_POPUP_MENU_H

#include <guichan.hpp>
#include "window.h"
#include "linkhandler.h"
#include "browserbox.h"
#include "../being.h"
#include "../floor_item.h"

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
         * Destructor.
         */
        ~PopupMenu();

        /**
         * Sets the visibility of popup
         */
        void setVisible(bool visible);

        /**
         * Shows the related popup menu specifies by the mouse click coords.
         */
        void showPopup(int mx, int my);

        /**
         * Draws updated popup menu
         */
        void draw(gcn::Graphics* graphics);

        /**
         * Handles link action.
         */
        void handleLink(const std::string& link);
        
    private:
        BrowserBox* browserBox;
        int mX, mY;
        
        Being* being;
        FloorItem* floorItem;
};

#endif
