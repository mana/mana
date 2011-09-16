/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

TabbedArea::TabbedArea() : gcn::TabbedArea(),
                           mTabsWidth(0),
                           mVisibleTabsWidth(0),
                           mTabScrollIndex(0)
{
    mWidgetContainer->setOpaque(false);
    addWidgetListener(this);

    mArrowButton[0] = new Button("", "shift_left", this);
    mArrowButton[1] = new Button("", "shift_right", this);
    if (!mArrowButton[0]->setButtonIcon("tab_arrows_left.png"))
        mArrowButton[0]->setCaption("<");
    if (!mArrowButton[1]->setButtonIcon("tab_arrows_right.png"))
        mArrowButton[1]->setCaption(">");

    add(mArrowButton[0]);
    add(mArrowButton[1]);

    widgetResized(NULL);
}

int TabbedArea::getNumberOfTabs() const
{
    return mTabs.size();
}

Tab *TabbedArea::getTab(const std::string &name) const
{
    TabContainer::const_iterator itr = mTabs.begin(), itr_end = mTabs.end();
    while (itr != itr_end)
    {
        if ((*itr).first->getCaption() == name)
            return static_cast<Tab*>((*itr).first);

        ++itr;
    }
    return NULL;
}

void TabbedArea::draw(gcn::Graphics *graphics)
{
    if (mTabs.empty())
        return;

    drawChildren(graphics);
}

gcn::Widget *TabbedArea::getWidget(const std::string &name) const
{
    TabContainer::const_iterator itr = mTabs.begin(), itr_end = mTabs.end();
    while (itr != itr_end)
    {
        if ((*itr).first->getCaption() == name)
            return (*itr).second;

        ++itr;
    }

    return NULL;
}

gcn::Widget *TabbedArea::getCurrentWidget()
{
    gcn::Tab *tab = getSelectedTab();

    if (tab)
        return getWidget(tab->getCaption());
    else
        return NULL;
}

void TabbedArea::addTab(gcn::Tab* tab, gcn::Widget* widget)
{
    gcn::TabbedArea::addTab(tab, widget);

    int width = getWidth() - 2 * getFrameSize();
    int height = getHeight() - 2 * getFrameSize() - mTabContainer->getHeight();
    widget->setSize(width, height);

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
            mSelectedTab = 0;
    }

    TabContainer::iterator iter;
    for (iter = mTabs.begin(); iter != mTabs.end(); iter++)
    {
        if (iter->first == tab)
        {
            mTabContainer->remove(tab);
            mTabs.erase(iter);
            break;
        }
    }

    std::vector<gcn::Tab*>::iterator iter2;
    for (iter2 = mTabsToDelete.begin(); iter2 != mTabsToDelete.end(); iter2++)
    {
        if (*iter2 == tab)
        {
            mTabsToDelete.erase(iter2);
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
        gcn::Tab *tab = dynamic_cast<gcn::Tab*>(widget);

        if (tab)
        {
            setSelectedTab(tab);
            requestFocus();
        }
    }
}

void TabbedArea::setSelectedTab(gcn::Tab *tab)
{
    gcn::TabbedArea::setSelectedTab(tab);

    Tab *newTab = dynamic_cast<Tab*>(tab);

    if (newTab)
        newTab->setCurrent();

    widgetResized(NULL);
}

void TabbedArea::widgetResized(const gcn::Event &event)
{
    int width = getWidth() - 2 * getFrameSize()
                - 2 * mWidgetContainer->getFrameSize();
    int height = getHeight() - 2 * getFrameSize() - mWidgetContainer->getY()
                 - 2 * mWidgetContainer->getFrameSize();
    mWidgetContainer->setSize(width, height);

    gcn::Widget *w = getCurrentWidget();
    if (w)
        w->setSize(width, height);

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
    mArrowButton[1]->setPosition(width - mArrowButton[1]->getWidth(), 0);

    updateArrowEnableState();
    adjustTabPositions();
}

void TabbedArea::updateTabsWidth()
{
    mTabsWidth = 0;
    for (TabContainer::const_iterator itr = mTabs.begin(), itr_end = mTabs.end();
         itr != itr_end; ++itr)
    {
        mTabsWidth += (*itr).first->getWidth();
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
    for (unsigned i = 0; i < mTabs.size(); ++i)
    {
        if (mTabs[i].first->getHeight() > maxTabHeight)
        {
            maxTabHeight = mTabs[i].first->getHeight();
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
    Widget* source = actionEvent.getSource();
    Tab* tab = dynamic_cast<Tab*>(source);

    if (tab)
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
    if (mTabsWidth > getWidth() - 4
        - mArrowButton[0]->getWidth()
        - mArrowButton[1]->getWidth())
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
    if (mVisibleTabsWidth < getWidth() - 4
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
