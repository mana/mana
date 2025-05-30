/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#pragma once

#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>
#include <guichan/widgets/container.hpp>
#include <guichan/widgets/tabbedarea.hpp>

#include "gui/widgets/button.h"

#include <memory>
#include <string>

class Tab;

/**
 * A tabbed area, the same as the guichan tabbed area in 0.8, but extended
 */
class TabbedArea final : public gcn::TabbedArea, public gcn::WidgetListener
{
    public:
        TabbedArea();

        /**
         * Draw the tabbed area.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Return how many tabs have been created.
         *
         * @todo Remove this method when upgrading to Guichan 0.9.0
         */
        int getNumberOfTabs() const;

        /**
         * Return tab with specified name as caption.
         */
        Tab *getTab(const std::string &name) const;

        /**
         * Returns the widget with the tab that has specified caption
         */
        gcn::Widget *getWidget(const std::string &name) const;

        /**
         * Returns the widget for the current tab
         */
        gcn::Widget *getCurrentWidget();

        /**
         * Add a tab. Overridden since it needs to size the widget.
         *
         * @param tab The tab widget for the tab.
         * @param widget The widget to view when the tab is selected.
         */
        void addTab(gcn::Tab* tab, gcn::Widget* widget) override;

        /**
         * Add a tab. Overridden since it needs to create an instance of Tab
         * instead of gcn::Tab.
         *
         * @param caption The Caption to display
         * @param widget The widget to show when tab is selected
         */
        void addTab(const std::string &caption, gcn::Widget *widget) override;

        /**
         * Override the remove tab function as it's broken in guichan 0.8.
         */
        void removeTab(gcn::Tab *tab) override;

        /**
         * Override the logic function since it's broken in guichan 0.8.
         */
        void logic() override;

        int getContainerHeight() const
        { return mWidgetContainer->getHeight(); }

        void setSelectedTab(unsigned int index) override
        { gcn::TabbedArea::setSelectedTab(index); }

        void setSelectedTab(gcn::Tab *tab) override;

        void widgetResized(const gcn::Event &event) override;

        void adjustTabPositions();

        void action(const gcn::ActionEvent& actionEvent) override;

        // Inherited from MouseListener

        void mousePressed(gcn::MouseEvent &mouseEvent) override;

    private:
        /** The tab arrows */
        std::unique_ptr<Button> mArrowButton[2];

        /** Check whether the arrow should be clickable */
        void updateArrowEnableState();

        /**
         * Update the overall width of all tab. Used to know whether the arrows
         * have to be drawn or not.
         */
        void updateTabsWidth();

        /**
         * The overall width of all tab.
         */
        int mTabsWidth = 0;

        /**
         * Update the overall width of visible tab. Used to know whether
         * the arrows have to be enable or not.
         */
        void updateVisibleTabsWidth();

        /**
         * The overall width of visible tab.
         */
        int mVisibleTabsWidth = 0;


        /**
         * The tab scroll index. When scrolling with the arrows, the tabs
         * must be displayed according to the current index.
         * So the first tab displayed may not be the first in the list.
         * @note the index must start at 0.
         */
        unsigned mTabScrollIndex = 0;
};
