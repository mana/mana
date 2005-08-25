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

#include "tabbedcontainer.h"

#include <sstream>

#include "button.h"

#define TABWIDTH 60
#define TABHEIGHT 20

TabbedContainer::TabbedContainer():
    mActiveContent(0)
{
}

TabbedContainer::~TabbedContainer()
{
    std::vector<gcn::Widget*>::iterator i = mTabs.begin();
    while (i != mTabs.end()) {
        remove(*i);
        delete (*i);
        i = mTabs.erase(i);
    }

    mContents.clear();
}

void TabbedContainer::addTab(gcn::Widget *widget, const std::string &caption)
{
    std::stringstream ss;

    Button *tab = new Button(caption);

    int tabNumber = mTabs.size();

    ss << tabNumber;
    tab->setEventId(ss.str());
    tab->setSize(TABWIDTH, TABHEIGHT);
    tab->addActionListener(this);
    add(tab, TABWIDTH * tabNumber, 0);

    mTabs.push_back(tab);

    mContents.push_back(widget);
    widget->setPosition(0, TABHEIGHT);

    // If this is the first tab in this container, make it visible
    if (!mActiveContent) {
        mActiveContent = widget;
        add(mActiveContent);
    }
}

void TabbedContainer::logic()
{
    if (mActiveContent) {
        mActiveContent->setSize(
                getWidth() - 2 * mActiveContent->getBorderSize(),
                getHeight() - TABHEIGHT - 2 * mActiveContent->getBorderSize());
    }

    Container::logic();
}

void TabbedContainer::action(const std::string &event)
{
    int tabNo;
    std::stringstream ss;
    gcn::Widget *newContent;

    ss << event;
    ss >> tabNo;

    if ((newContent = mContents[tabNo])) {
        if (mActiveContent) {
            remove(mActiveContent);
        }
        mActiveContent = newContent;
        add(newContent);
    }
}

void TabbedContainer::setOpaque(bool opaque)
{
    Container::setOpaque(opaque);
}
