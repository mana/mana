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

#include "gui/widgets/tabbedarea.h"

#include "gui/widgets/tab.h"

#include <guichan/widgets/container.hpp>

TabbedArea::TabbedArea()
{
    mWidgetContainer->setOpaque(false);
    addWidgetListener(this);

    mArrowButton[0] = new Button(std::string(), "shift_left", this);
    mArrowButton[1] = new Button(std::string(), "shift_right", this);
    mArrowButton[0]->setButtonIcon("tab_arrows_left.png");
    mArrowButton[1]->setButtonIcon("tab_arrows_right.png");

    add(mArrowButton[0]);
    add(mArrowButton[1]);

    widgetResized(nullptr);
}

int TabbedArea::getNumberOfTabs() const
{
    return mTabs.size();
}

Tab *TabbedArea::getTab(const std::string &name) const
{
    for (const auto &[tab, _] : mTabs)
    {
        if (tab->getCaption() == name)
            return static_cast<Tab*>(tab);
    }
    return nullptr;
}

void TabbedArea::draw(gcn::Graphics *graphics)
{
    if (mTabs.empty())
        return;

    drawChildren(graphics);
}

gcn::Widget *TabbedArea::getWidget(const std::string &name) const
{
    for (const auto &[tab, widget] : mTabs)
    {
        if (tab->getCaption() == name)
            return widget;
    }

    return nullptr;
}

gcn::Widget *TabbedArea::getCurrentWidget()
{
    if (gcn::Tab *tab = getSelectedTab())
        return getWidget(tab->getCaption());

    return nullptr;
}

void TabbedArea::addTab(gcn::Tab* tab, gcn::Widget* widget)
{
    gcn::TabbedArea::addTab(tab, widget);

    int frameSize = widget->getFrameSize();
    widget->setPosition(frameSize, frameSize);
    widget->setSize(mWidgetContainer->getWidth() - frameSize * 2,
                    mWidgetContainer->getHeight() - frameSize * 2);

    updateTabsWidth();
    updateArrowEnableState();
}

void TabbedArea::addTab(const std::string &caption, gcn::Widget *widget)
{
    Tab *tab = new Tab;
    tab->setCaption(caption);
    mTabsToDelete.push_back(tab);

    addTab(tab, widget);
}

void TabbedArea::removeTab(Tab *tab)
{
    if (tab == mSelectedTab)
    {
        if (getNumberOfTabs() > 1)
            setSelectedTab(std::max(0, getSelectedTabIndex() - 1));
        else
            mSelectedTab = nullptr;
    }

    for (auto iter = mTabs.begin(); iter != mTabs.end(); iter++)
    {
        if (iter->first == tab)
        {
            mTabContainer->remove(tab);
            mTabs.erase(iter);
            break;
        }
    }

    for (auto iter = mTabsToDelete.begin(); iter != mTabsToDelete.end(); iter++)
    {
        if (*iter == tab)
        {
            mTabsToDelete.erase(iter);
            delete tab;
            break;
        }
    }

    adjustSize();
    updateTabsWidth();
    adjustTabPositions();
}

void TabbedArea::logic()
{
    logicChildren();
}

void TabbedArea::mousePressed(gcn::MouseEvent &mouseEvent)
{
    if (mouseEvent.isConsumed())
        return;

    if (mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
        gcn::Widget *widget = mTabContainer->getWidgetAt(mouseEvent.getX(),
                                                         mouseEvent.getY());

        if (auto *tab = dynamic_cast<gcn::Tab*>(widget))
        {
            setSelectedTab(tab);
            requestFocus();
        }
    }
}

void TabbedArea::setSelectedTab(gcn::Tab *tab)
{
    gcn::TabbedArea::setSelectedTab(tab);

    if (Tab *newTab = dynamic_cast<Tab*>(tab))
        newTab->setCurrent();

    widgetResized(nullptr);
}

void TabbedArea::widgetResized(const gcn::Event &event)
{
    adjustSize();

    if (gcn::Widget *w = getCurrentWidget())
    {
        int frameSize = w->getFrameSize();
        w->setSize(mWidgetContainer->getWidth() - frameSize * 2,
                   mWidgetContainer->getHeight() - frameSize * 2);
    }

    // Check whether there is room to show more tabs now.
    int innerWidth = getWidth() - 4 - mArrowButton[0]->getWidth()
        - mArrowButton[1]->getWidth();
    int newWidth = mVisibleTabsWidth;
    while (mTabScrollIndex && newWidth < innerWidth)
    {
        newWidth += mTabs[mTabScrollIndex - 1].first->getWidth();
        if (newWidth < innerWidth)
            --mTabScrollIndex;
    }

    // Move the right arrow to fit the windows content.
    mArrowButton[1]->setPosition(getWidth() - mArrowButton[1]->getWidth(), 0);

    updateArrowEnableState();
    adjustTabPositions();
}

void TabbedArea::updateTabsWidth()
{
    mTabsWidth = 0;
    for (const auto &[tab, _] : mTabs)
    {
        mTabsWidth += tab->getWidth();
    }
    updateVisibleTabsWidth();
}

void TabbedArea::updateVisibleTabsWidth()
{
    mVisibleTabsWidth = 0;
    for (unsigned int i = mTabScrollIndex; i < mTabs.size(); ++i)
    {
        mVisibleTabsWidth += mTabs[i].first->getWidth();
    }
}

void TabbedArea::adjustTabPositions()
{
    int maxTabHeight = 0;
    for (auto &tab : mTabs)
    {
        if (tab.first->getHeight() > maxTabHeight)
        {
            maxTabHeight = tab.first->getHeight();
        }
    }

    int x = mArrowButton[0]->isVisible() ? mArrowButton[0]->getWidth() : 0;
    for (unsigned i = mTabScrollIndex; i < mTabs.size(); ++i)
    {
        gcn::Tab* tab = mTabs[i].first;
        tab->setPosition(x, maxTabHeight - tab->getHeight());
        x += tab->getWidth();
    }

    // If the tabs are scrolled, we hide them away.
    if (mTabScrollIndex > 0)
    {
        x = 0;
        for (unsigned i = 0; i < mTabScrollIndex; ++i)
        {
            gcn::Tab* tab = mTabs[i].first;
            x -= tab->getWidth();
            tab->setPosition(x, maxTabHeight - tab->getHeight());
        }
    }
}

void TabbedArea::action(const gcn::ActionEvent& actionEvent)
{
    Widget *source = actionEvent.getSource();

    if (Tab *tab = dynamic_cast<Tab*>(source))
    {
        setSelectedTab(tab);
    }
    else
    {
        if (actionEvent.getId() == "shift_left")
        {
            if (mTabScrollIndex)
                --mTabScrollIndex;
        }
        else if (actionEvent.getId() == "shift_right")
        {
            if (mTabScrollIndex < mTabs.size() - 1)
                ++mTabScrollIndex;
        }
        adjustTabPositions();

        updateArrowEnableState();
    }
}

void TabbedArea::updateArrowEnableState()
{
    updateTabsWidth();
    if (mTabsWidth > getWidth() - 2)
    {
        mArrowButton[0]->setVisible(true);
        mArrowButton[1]->setVisible(true);
    }
    else
    {
        mArrowButton[0]->setVisible(false);
        mArrowButton[1]->setVisible(false);
        mTabScrollIndex = 0;
    }

    // Left arrow consistency check
    if (!mTabScrollIndex)
        mArrowButton[0]->setEnabled(false);
    else
        mArrowButton[0]->setEnabled(true);

    // Right arrow consistency check
    if (mVisibleTabsWidth < getWidth() - 2
        - mArrowButton[0]->getWidth()
        - mArrowButton[1]->getWidth())
    {
        mArrowButton[1]->setEnabled(false);
    }
    else
    {
        mArrowButton[1]->setEnabled(true);
    }
}
