/*
 *  The Mana World
 *  Copyright (C) 2007  The Mana World Development Team
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

#include "flowcontainer.h"

FlowContainer::FlowContainer(int boxWidth, int boxHeight):
    mBoxWidth(boxWidth), mBoxHeight(boxHeight),
    mGridWidth(1), mGridHeight(1)
{
    addWidgetListener(this);
}

void FlowContainer::widgetResized(const gcn::Event &event)
{
    if (getWidth() < mBoxWidth)
    {
        setWidth(mBoxWidth);
        return;
    }

    int itemCount = mWidgets.size();

    mGridWidth = getWidth() / mBoxWidth;

    if (mGridWidth < 1)
        mGridWidth = 1;

    mGridHeight = itemCount / mGridWidth;

    if (itemCount % mGridWidth != 0 || mGridHeight < 1)
        ++mGridHeight;

    int height = mGridHeight * mBoxHeight;

    if (getHeight() != height)
    {
        setHeight(height);
        return;
    }

    int i = 0;
    height = 0;
    for (WidgetList::iterator it = mWidgets.begin(); it != mWidgets.end(); it++)
    {
        int x = i % mGridWidth * mBoxWidth;
        (*it)->setPosition(x, height);

        i++;

        if (i % mGridWidth == 0)
            height += mBoxHeight;
    }
}

void FlowContainer::add(gcn::Widget *widget)
{
    Container::add(widget);
    widget->setSize(mBoxWidth, mBoxHeight);
    widgetResized(NULL);
}
