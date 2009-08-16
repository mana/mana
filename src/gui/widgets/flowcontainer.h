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

#ifndef FLOWCONTAINER_H
#define FLOWCONTAINER_H

#include "container.h"

#include <guichan/widgetlistener.hpp>

/**
 * A container that arranges its contents like words on a page.
 *
 * \ingroup GUI
 */
class FlowContainer : public Container,
                      public gcn::WidgetListener
{
    public:
        /**
         * Constructor. Initializes the shortcut container.
         */
        FlowContainer(int boxWidth, int boxHeight);

        /**
         * Destructor.
         */
        ~FlowContainer() {}

        /**
         * Invoked when a widget changes its size. This is used to determine
         * the new height of the container.
         */
        void widgetResized(const gcn::Event &event);

        int getBoxWidth() const
        { return mBoxWidth; }

        int getBoxHeight() const
        { return mBoxHeight; }

        void add(gcn::Widget *widget);

    private:
        int mBoxWidth;
        int mBoxHeight;
        int mGridWidth, mGridHeight;
};

#endif
