/*
 *  The Mana Client
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
        FlowContainer(int boxWidth, int boxHeight);

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
