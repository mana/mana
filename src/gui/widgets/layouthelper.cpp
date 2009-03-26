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

#include "gui/widgets/layouthelper.h"

LayoutHelper::LayoutHelper(gcn::Container *container):
    mContainer(container)
{
    mContainer->addWidgetListener(this);
}

LayoutHelper::~LayoutHelper()
{
    mContainer->removeWidgetListener(this);
}

Layout &LayoutHelper::getLayout()
{
    return mLayout;
}

LayoutCell &LayoutHelper::place(int x, int y, gcn::Widget *wg, int w, int h)
{
    mContainer->add(wg);
    return mLayout.place(wg, x, y, w, h);
}

ContainerPlacer LayoutHelper::getPlacer(int x, int y)
{
    return ContainerPlacer(mContainer, &mLayout.at(x, y));
}

void LayoutHelper::reflowLayout(int w, int h)
{
    mLayout.reflow(w, h);
    mContainer->setSize(w, h);
}

void LayoutHelper::widgetResized(const gcn::Event &event)
{
    const gcn::Rectangle area = mContainer->getChildrenArea();
    int w = area.width;
    int h = area.height;
    mLayout.reflow(w, h);
}
