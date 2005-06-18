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

#include "vbox.h"

VBox::VBox()
    : Box()
{
}

VBox::~VBox()
{
}

void VBox::draw(gcn::Graphics *graphics)
{
    int widgetCount = mWidgets.size();
    int childWidth = getWidth();
    if (widgetCount == 0)
	return;
    int childHeight = getHeight() / widgetCount;

    int i = 0;
    for (WidgetIterator w = mWidgets.begin(); w != mWidgets.end(); w++) {
	(*w)->setPosition(0, childHeight * i - padding);
        (*w)->setSize(childWidth, childHeight);
        i++;
    }
    gcn::Container::draw(graphics);
}
