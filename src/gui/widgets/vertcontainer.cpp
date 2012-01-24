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

#include "gui/widgets/vertcontainer.h"

VertContainer::VertContainer(int spacing):
        mSpacing(spacing),
        mCount(0)
{
    addWidgetListener(this);
}

void VertContainer::add(gcn::Widget *widget)
{
    Container::add(widget);
    widget->setPosition(0, mCount * mSpacing);
    widget->setSize(getWidth(), mSpacing);
    mCount++;
    setHeight(mCount * mSpacing);
}

void VertContainer::clear()
{
    Container::clear();

    mCount = 0;
}

void VertContainer::widgetResized(const gcn::Event &event)
{
    for (WidgetListIterator it = mWidgets.begin(); it != mWidgets.end(); it++)
    {
        (*it)->setWidth(getWidth());
    }
}
