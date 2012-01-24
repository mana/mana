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

#include "gui/widgets/windowcontainer.h"

#include "gui/widgets/window.h"

#include "utils/dtor.h"

WindowContainer *windowContainer = NULL;

void WindowContainer::logic()
{
    delete_all(mDeathList);
    mDeathList.clear();

    gcn::Container::logic();
}

void WindowContainer::scheduleDelete(gcn::Widget *widget)
{
    mDeathList.push_back(widget);
}

void WindowContainer::adjustAfterResize(int oldScreenWidth,
                                        int oldScreenHeight)
{
    for (WidgetListIterator i = mWidgets.begin(); i != mWidgets.end(); ++i)
        if (Window *window = dynamic_cast<Window*>(*i))
            window->adjustPositionAfterResize(oldScreenWidth, oldScreenHeight);
}
