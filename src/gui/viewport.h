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

#ifndef _TMW_VIEWPORT_H_
#define _TMW_VIEWPORT_H_

#include <guichan/mouselistener.hpp>

#include "windowcontainer.h"

#include "../configlistener.h"

class Map;
class Being;
class FloorItem;
class Item;
class PopupMenu;

/**
 * The viewport on the map. Displays the current map and handles mouse input
 * and the popup menu.
 *
 * TODO: This class is planned to be extended to allow floating widgets on top
 * of it such as NPC messages, which are positioned using map pixel
 * coordinates.
 */
class Viewport : public WindowContainer, public gcn::MouseListener,
    public ConfigListener
{
    public:
        /**
         * Constructor.
         */
        Viewport();

        /**
         * Destructor.
         */
        ~Viewport();

        /**
         * Sets the map displayed by the viewport.
         */
        void
        setMap(Map *map);

        /**
         * Draws the viewport.
         */
        void
        draw(gcn::Graphics *graphics);

        /**
         * Implements player to keep following mouse.
         */
        void
        logic();

        /**
         * Toggles whether the path debug graphics are shown
         */
        void toggleDebugPath() { mShowDebugPath = !mShowDebugPath; }

        /**
         * Handles mouse press on map.
         */
        void
        mousePress(int mx, int my, int button);

        /**
         * Handles mouse move on map
         */
        void
        mouseMotion(int mx, int my);

        /**
         * Handles mouse button release on map.
         */
        void
        mouseRelease(int mx, int my, int button);

        /**
         * Shows a popup for an item.
         * TODO Find some way to get rid of Item here
         */
        void showPopup(int x, int y, Item *item);

        /**
         * A relevant config option changed.
         */
        void
        optionChanged(const std::string &name);

    private:
        /**
         * Shows a popup for a floor item.
         * TODO Find some way to get rid of FloorItem here
         */
        void showPopup(int x, int y, FloorItem *floorItem);

        /**
         * Shows a popup for a being.
         * TODO Find some way to get rid of Being here
         */
        void showPopup(int x, int y, Being *being);


        Map *mMap;                 /**< The current map. */

        int mScrollRadius;
        int mScrollLaziness;
        float mViewX;              /**< Current viewpoint in pixels. */
        float mViewY;              /**< Current viewpoint in pixels. */
        int mCameraX;
        int mCameraY;
        bool mShowDebugPath;       /**< Show a path from player to pointer. */

        bool mPlayerFollowMouse;
        int mWalkTime;

        PopupMenu *mPopupMenu;     /**< Popup menu. */
        bool mPopupActive;
};

#endif
