/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef LAYOUTHELPER_H
#define LAYOUTHELPER_H

#include "gui/widgets/layout.h"

#include <guichan/widgetlistener.hpp>

/**
 * A helper class for adding a layout to a Guichan container widget. The layout
 * will register itself as a widget listener and relayout the widgets in the
 * container dynamically on resize.
 */
class LayoutHelper : public gcn::WidgetListener
{
    public:
        /**
         * Constructor.
         */
        LayoutHelper(gcn::Container *container);

        /**
         * Destructor.
         */
        ~LayoutHelper();

        /**
         * Gets the layout handler.
         */
        Layout &getLayout();

        /**
         * Computes the position of the widgets according to the current
         * layout. Resizes the managed container so that the layout fits.
         *
         * @note This function is meant to be called with fixed-size
         * containers.
         *
         * @param w if non-zero, force the container to this width.
         * @param h if non-zero, force the container to this height.
         */
        void reflowLayout(int w = 0, int h = 0);

        /**
         * Adds a widget to the container and sets it at given cell.
         */
        LayoutCell &place(int x, int y, gcn::Widget *, int w = 1, int h = 1);

        /**
         * Returns a proxy for adding widgets in an inner table of the layout.
         */
        ContainerPlacer getPlacer(int x, int y);

        /**
         * Called whenever the managed container changes size.
         */
        void widgetResized(const gcn::Event &event);

    private:
        Layout mLayout;              /**< Layout handler */
        gcn::Container *mContainer;  /**< Managed container */
};

#endif // LAYOUTHELPER_H
