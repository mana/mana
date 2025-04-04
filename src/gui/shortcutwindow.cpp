/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
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

#include "gui/shortcutwindow.h"

#include "gui/setup.h"

#include "gui/widgets/layout.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/shortcutcontainer.h"

static constexpr int GRAB_MARGIN = 4;

ShortcutWindow::ShortcutWindow(const std::string &title,
                               ShortcutContainer *content)
    : Window(SkinType::ToolWindow, std::string())
{
    setWindowName(title);
    // no title presented, title bar gets some extra space so window can be moved.
    setTitleBarHeight(getPadding() + GRAB_MARGIN);
    setShowTitle(false);
    setResizable(true);
    setDefaultVisible(false);
    setSaveVisible(true);
    setupWindow->registerWindowForReset(this);

    auto scrollArea = new ScrollArea(content);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setOpaque(false);

    const int border = (getPadding() + content->getFrameSize()) * 2;
    setMinWidth(content->getBoxWidth() + border);
    setMinHeight(content->getBoxHeight() + border + GRAB_MARGIN);
    setMaxWidth(content->getBoxWidth() * content->getMaxItems() + border);
    setMaxHeight(content->getBoxHeight() * content->getMaxItems() + border + GRAB_MARGIN);

    setDefaultSize(getMinWidth(), getMaxHeight(), ImageRect::LOWER_RIGHT);

    place(0, 0, scrollArea, 5, 5).setPadding(0);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);
    layout.setMargin(0);

    loadWindowState();
}
