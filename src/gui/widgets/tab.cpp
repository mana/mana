/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "gui/widgets/tab.h"

#include "graphics.h"

#include "gui/gui.h"
#include "gui/widgets/tabbedarea.h"

#include "resources/theme.h"

#include <guichan/widgets/label.hpp>

Tab::Tab() :
    mTabColor(&Theme::getThemeColor(Theme::TAB))
{
    init();
}

void Tab::init()
{
    setFocusable(false);
    setFrameSize(0);
}

void Tab::draw(gcn::Graphics *graphics)
{
    WidgetState state(this);
    if (mHasMouse)
        state.flags |= STATE_HOVERED;
    if (mTabbedArea && mTabbedArea->isTabSelected(this))
        state.flags |= STATE_SELECTED;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), SkinType::Tab, state);

    // if tab is selected, it doesnt need to highlight activity
    if (state.flags & STATE_SELECTED)
        mFlash = false;

    if (mFlash)
        mLabel->setForegroundColor(Theme::getThemeColor(Theme::TAB_FLASH));
    else
        mLabel->setForegroundColor(*mTabColor);

    // draw label
    drawChildren(graphics);
}

void Tab::setTabColor(const gcn::Color *color)
{
    mTabColor = color;
}

void Tab::setFlash(bool flash)
{
    mFlash = flash;
}
