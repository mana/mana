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

#include "scrollarea.h"
#include "shortcutcontainer.h"
#include "shortcutwindow.h"

#include "../configuration.h"

static const int SCROLL_PADDING = 0;

ShortcutWindow::ShortcutWindow(const char *title, ShortcutContainer *content)
{
    setWindowName(title);
    // no title presented, title bar is padding so window can be moved.
    gcn::Window::setTitleBarHeight(gcn::Window::getPadding());
    setShowTitle(false);
    setResizable(true);

    mItems = content;

    const int border = SCROLL_PADDING * 2 + getPadding() * 2;
    setMinWidth(mItems->getBoxWidth() + border);
    setMinHeight(mItems->getBoxHeight() + border);
    setMaxWidth(mItems->getBoxWidth() * mItems->getMaxItems() + border);
    setMaxHeight(mItems->getBoxHeight() * mItems->getMaxItems() + border);

    const int width = (int) config.getValue("screenwidth", 800);
    const int height = (int) config.getValue("screenheight", 600);

    setDefaultSize(width - mItems->getBoxWidth() - border, 
                   height - (mItems->getBoxHeight() * mItems->getMaxItems()) - 
                   border, mItems->getBoxWidth() + border, 
                   (mItems->getBoxHeight() * mItems->getMaxItems()) + border);

    mScrollArea = new ScrollArea(mItems);
    mScrollArea->setPosition(SCROLL_PADDING, SCROLL_PADDING);
    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mScrollArea->setOpaque(false);

    add(mScrollArea);

    loadWindowState();
}

ShortcutWindow::~ShortcutWindow()
{
    delete mItems;
    delete mScrollArea;
}

void ShortcutWindow::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);

    const gcn::Rectangle &area = getChildrenArea();

    mScrollArea->setSize(
            area.width - SCROLL_PADDING,
            area.height - SCROLL_PADDING);
}
