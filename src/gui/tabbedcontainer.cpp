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

#include "button.h"

#include "../utils/tostring.h"

#define TABWIDTH 60
#define TABHEIGHT 20

TabbedContainer::TabbedContainer():
    mActiveContent(0)
{
}

TabbedContainer::~TabbedContainer()
{
    for (WidgetIterator i = mTabs.begin(); i != mTabs.end(); i++) {
        remove(*i);
        delete (*i);
    }

    mTabs.clear();
    mContents.clear();
}

void TabbedContainer::addTab(gcn::Widget *widget, const std::string &caption)
{
    int tabNumber = mTabs.size();

    Button *tab = new Button(caption, toString(tabNumber), this);

    tab->setSize(TABWIDTH, TABHEIGHT);
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
    std::stringstream ss(event);
    int tabNo;
    ss >> tabNo;

    gcn::Widget *newContent = mContents[tabNo];
    if (newContent) {
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
