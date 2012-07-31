/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#ifndef GUI_CONTAINER_H
#define GUI_CONTAINER_H

#include <guichan/widgets/container.hpp>

class ContainerPlacer;
class Layout;
class LayoutCell;
class LayoutHelper;

/**
 * A widget container.
 *
 * The main difference between the standard Guichan container and this one is
 * that childs added to this container are automatically deleted when the
 * container is deleted.
 *
 * This container is also non-opaque by default.
 */
class Container : public gcn::Container
{
    public:
        Container();
        ~Container();

        /**
         * Gets the layout handler for this container.
         */
        Layout &getLayout();

        /**
         * Adds a widget to the container and sets it at given cell.
         */
        LayoutCell &place(int x, int y, gcn::Widget *wg, int w = 1, int h = 1);

        /**
         * Returns a proxy for adding widgets in an inner table of the layout.
         */
        ContainerPlacer getPlacer(int x, int y);

        /**
         * Updates the layout to match the available size in the container
         * (happens automatically on resizes).
         */
        void updateLayout();

    private:
        LayoutHelper *mLayoutHelper;
};

#endif
