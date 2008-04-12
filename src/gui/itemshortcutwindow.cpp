/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
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

#include "itemshortcutwindow.h"

#include "itemshortcutcontainer.h"
#include "scrollarea.h"

static const int SCROLL_PADDING = 0;

ItemShortcutWindow::ItemShortcutWindow()
{
    setWindowName("itemShortcut");
    // no title presented, title bar is padding so window can be moved.
    gcn::Window::setTitleBarHeight(gcn::Window::getPadding());
    setShowTitle(false);
    setResizable(true);
    setDefaultSize(758, 174, 42, 426);

    mItems = new ItemShortcutContainer();

    const int border = SCROLL_PADDING * 2 + getPadding() * 2;
    setMinWidth(mItems->getBoxWidth() + border);
    setMinHeight(mItems->getBoxHeight() + border);
    setMaxWidth(mItems->getBoxWidth() * mItems->getMaxItems() + border);
    setMaxHeight(mItems->getBoxHeight() * mItems->getMaxItems() + border);

    mScrollArea = new ScrollArea(mItems);
    mScrollArea->setPosition(SCROLL_PADDING, SCROLL_PADDING);
    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mScrollArea->setOpaque(false);

    add(mScrollArea);

    loadWindowState();
}

ItemShortcutWindow::~ItemShortcutWindow()
{
    delete mItems;
    delete mScrollArea;
}

void ItemShortcutWindow::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);

    const gcn::Rectangle &area = getChildrenArea();

    mScrollArea->setSize(
            area.width - SCROLL_PADDING,
            area.height - SCROLL_PADDING);
}
