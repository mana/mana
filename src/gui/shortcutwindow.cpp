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

#include "gui/shortcutwindow.h"

#include "gui/shortcutcontainer.h"

#include "gui/widgets/layout.h"
#include "gui/widgets/scrollarea.h"

#include "configuration.h"

static const int SCROLL_PADDING = 0;

int ShortcutWindow::mInstances = 0;

ShortcutWindow::ShortcutWindow(const std::string &title,
                               ShortcutContainer *content)
{
    setWindowName(title);
    // no title presented, title bar is padding so window can be moved.
    gcn::Window::setTitleBarHeight(gcn::Window::getPadding());
    setShowTitle(false);
    setResizable(true);
    setDefaultVisible(false);
    setSaveVisible(true);

    mItems = content;

    const int border = SCROLL_PADDING * 2 + getPadding() * 2;
    setMinWidth(mItems->getBoxWidth() + border);
    setMinHeight(mItems->getBoxHeight() + border);
    setMaxWidth(mItems->getBoxWidth() * mItems->getMaxItems() + border);
    setMaxHeight(mItems->getBoxHeight() * mItems->getMaxItems() + border);

    setDefaultSize(mItems->getBoxWidth() + border, (mItems->getBoxHeight() *
                   mItems->getMaxItems()) + border, ImageRect::LOWER_RIGHT,
                   mInstances * mItems->getBoxWidth(), 0);

    mInstances++;

    mScrollArea = new ScrollArea(mItems);
    mScrollArea->setPosition(SCROLL_PADDING, SCROLL_PADDING);
    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mScrollArea->setOpaque(false);

    place(0, 0, mScrollArea, 5, 5).setPadding(0);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);
    layout.setMargin(0);

    loadWindowState();
}

ShortcutWindow::~ShortcutWindow()
{
    delete mItems;
}
