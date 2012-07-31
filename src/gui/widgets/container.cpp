/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "gui/widgets/container.h"

#include "gui/widgets/layouthelper.h"

Container::Container():
    mLayoutHelper(0)
{
    setOpaque(false);
}

Container::~Container()
{
    while (!mWidgets.empty())
        delete mWidgets.front();

    delete mLayoutHelper;
}

Layout &Container::getLayout()
{
    if (!mLayoutHelper)
        mLayoutHelper = new LayoutHelper(this);
    return mLayoutHelper->getLayout();
}

LayoutCell &Container::place(int x, int y, gcn::Widget *wg, int w, int h)
{
    add(wg);
    return getLayout().place(wg, x, y, w, h);
}

ContainerPlacer Container::getPlacer(int x, int y)
{
    return ContainerPlacer(this, &getLayout().at(x, y));
}

void Container::updateLayout()
{
    const gcn::Rectangle area = getChildrenArea();
    int w = area.width;
    int h = area.height;
    getLayout().reflow(w, h);
}
