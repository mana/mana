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
#include "gui/widgets/label.h"
#include "gui/widgets/tabbedarea.h"

#include "resources/theme.h"

#include <guichan/widgets/label.hpp>

Tab::Tab()
{
    setFocusable(false);

    // Replace the label with customized version
    delete mLabel;
    mLabel = new Label();
    add(mLabel);

    auto &skin = gui->getTheme()->getSkin(SkinType::Tab);
    setFrameSize(skin.frameSize);
    mPadding = skin.padding;
    mLabel->setPosition(mPadding, mPadding);
}

void Tab::setCaption(const std::string &caption)
{
    mLabel->setCaption(caption);
    mLabel->adjustSize();

    setSize(mLabel->getWidth() + mPadding * 2,
            mLabel->getHeight() + mPadding * 2);

    if (mTabbedArea)
        static_cast<TabbedArea*>(mTabbedArea)->adjustTabPositions();
}

void Tab::draw(gcn::Graphics *graphics)
{
    if (getFrameSize() == 0)
        drawFrame(graphics);

    // if tab is selected, it doesnt need to highlight activity
    if (mTabbedArea && mTabbedArea->isTabSelected(this))
        mFlash = false;

    uint8_t flags = 0;
    if (mHasMouse)
        flags |= STATE_HOVERED;
    if (mTabbedArea && mTabbedArea->isTabSelected(this))
        flags |= STATE_SELECTED;

    auto &skin = gui->getTheme()->getSkin(SkinType::Tab);
    if (auto state = skin.getState(flags))
    {
        gcn::Color foregroundColor = state->textFormat.color;
        if (mFlash)
            foregroundColor = Theme::getThemeColor(Theme::TAB_FLASH);
        else if (mTabColor)
            foregroundColor = *mTabColor;

        auto label = static_cast<Label*>(mLabel);
        label->setForegroundColor(foregroundColor);
        label->setOutlineColor(state->textFormat.outlineColor);
        label->setShadowColor(state->textFormat.shadowColor);
    }

    // draw label
    drawChildren(graphics);
}

void Tab::drawFrame(gcn::Graphics *graphics)
{
    WidgetState state(this);
    state.width += getFrameSize() * 2;
    state.height += getFrameSize() * 2;
    if (mHasMouse)
        state.flags |= STATE_HOVERED;
    if (mTabbedArea && mTabbedArea->isTabSelected(this))
        state.flags |= STATE_SELECTED;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), SkinType::Tab, state);
}

void Tab::setTabColor(const gcn::Color *color)
{
    mTabColor = color;
}

void Tab::setFlash(bool flash)
{
    mFlash = flash;
}
