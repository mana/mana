/*
 *  The Mana World
 *  Copyright 2008 The Mana World Development Team
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
 *  $Id: $
 */

#ifndef _TMW_TABBEDAREA_H
#define _TMW_TABBEDAREA_H

#include <guichan/widget.hpp>
#include <guichan/widgets/tab.hpp>
#include <guichan/widgets/tabbedarea.hpp>

#include <string>

/**
 * A tabbed area, the same as the guichan tabbed area in 0.8, but extended
 */
class TabbedArea : public gcn::TabbedArea
{
    public:
        /**
         * Constructor.
         */
        TabbedArea();

        /**
         * Draw the tabbed area.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Return how many tabs have been created
         */
        int getNumberOfTabs();

        /**
         * Return tab with specified name as caption
         */
        gcn::Tab* getTab(const std::string &name);

        /**
         * Returns the widget with the tab that has specified caption
         */
        gcn::Widget* getWidget(const std::string &name);

        /**
         * Overload the remove tab function as its broken in guichan 0.8
         */
        void removeTab(gcn::Tab *tab);

        /**
         * Overload adjustSize as it seems faulty
         */
        void adjustSize();
};

#endif

