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

#include "gui/widgets/scrollarea.h"

#include "graphics.h"

#include "gui/gui.h"

ScrollArea::ScrollArea()
{
    init();
}

ScrollArea::ScrollArea(gcn::Widget *widget):
    gcn::ScrollArea(widget)
{
    init();
}

ScrollArea::~ScrollArea()
{
    delete getContent();
}

void ScrollArea::init()
{
    // Draw background by default
    setOpaque(true);

    auto theme = gui->getTheme();

    int minWidth = theme->getSkin(SkinType::ScrollAreaVBar).getMinWidth();
    if (minWidth > 0)
        setScrollbarWidth(minWidth);

    if (auto content = getContent())
        content->setFrameSize(theme->getSkin(SkinType::ScrollArea).padding);

    // The base color is only used when rendering a square in the corner where
    // the scrollbars meet. We disable rendering of this square by setting the
    // base color to transparent.
    setBaseColor(gcn::Color(0, 0, 0, 0));

    setUpButtonScrollAmount(5);
    setDownButtonScrollAmount(5);
    setLeftButtonScrollAmount(5);
    setRightButtonScrollAmount(5);
}

void ScrollArea::logic()
{
    if (!isVisible())
        return;

    gcn::ScrollArea::logic();
    gcn::Widget *content = getContent();

    // When no scrollbar in a certain direction, adapt content size to match
    // the content dimension exactly.
    if (content)
    {
        if (getHorizontalScrollPolicy() == gcn::ScrollArea::SHOW_NEVER)
        {
            content->setWidth(getChildrenArea().width -
                    2 * content->getFrameSize());
        }
        if (getVerticalScrollPolicy() == gcn::ScrollArea::SHOW_NEVER)
        {
            content->setHeight(getChildrenArea().height -
                    2 * content->getFrameSize());
        }
    }

    if (mUpButtonPressed)
    {
        setVerticalScrollAmount(getVerticalScrollAmount() -
                                mUpButtonScrollAmount);
    }
    else if (mDownButtonPressed)
    {
        setVerticalScrollAmount(getVerticalScrollAmount() +
                                mDownButtonScrollAmount);
    }
    else if (mLeftButtonPressed)
    {
        setHorizontalScrollAmount(getHorizontalScrollAmount() -
                                  mLeftButtonScrollAmount);
    }
    else if (mRightButtonPressed)
    {
        setHorizontalScrollAmount(getHorizontalScrollAmount() +
                                  mRightButtonScrollAmount);
    }
}

void ScrollArea::draw(gcn::Graphics *graphics)
{
    if (getFrameSize() == 0)
        drawFrame(graphics);

    gcn::ScrollArea::draw(graphics);
}

void ScrollArea::drawFrame(gcn::Graphics *graphics)
{
    if (!mOpaque)
        return;

    const int bs = getFrameSize();

    WidgetState state(this);
    state.width += bs * 2;
    state.height += + bs * 2;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), SkinType::ScrollArea, state);
}

void ScrollArea::setOpaque(bool opaque)
{
    mOpaque = opaque;

    auto &skin = gui->getTheme()->getSkin(SkinType::ScrollArea);
    setFrameSize(mOpaque ? skin.frameSize : 0);
}

void ScrollArea::drawBackground(gcn::Graphics *graphics)
{
    // background is drawn as part of the frame instead
}

void ScrollArea::drawUpButton(gcn::Graphics *graphics)
{
    drawButton(graphics, SkinType::ButtonUp, mUpButtonPressed, getUpButtonDimension());
}

void ScrollArea::drawDownButton(gcn::Graphics *graphics)
{
    drawButton(graphics, SkinType::ButtonDown, mDownButtonPressed, getDownButtonDimension());
}

void ScrollArea::drawLeftButton(gcn::Graphics *graphics)
{
    drawButton(graphics, SkinType::ButtonLeft, mLeftButtonPressed, getLeftButtonDimension());
}

void ScrollArea::drawRightButton(gcn::Graphics *graphics)
{
    drawButton(graphics, SkinType::ButtonRight, mRightButtonPressed, getRightButtonDimension());
}

void ScrollArea::drawVBar(gcn::Graphics *graphics)
{
    WidgetState state(getVerticalBarDimension());
    if (mHasMouse && (mX > (getWidth() - getScrollbarWidth())))
        state.flags |= STATE_HOVERED;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), SkinType::ScrollAreaVBar, state);
}

void ScrollArea::drawHBar(gcn::Graphics *graphics)
{
    WidgetState state(getHorizontalBarDimension());
    if (mHasMouse && (mY > (getHeight() - getScrollbarWidth())))
        state.flags |= STATE_HOVERED;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), SkinType::ScrollAreaHBar, state);
}

void ScrollArea::drawVMarker(gcn::Graphics *graphics)
{
    WidgetState state(getVerticalMarkerDimension());
    if (mHasMouse && (mX > (getWidth() - getScrollbarWidth())))
        state.flags |= STATE_HOVERED;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), SkinType::ScrollAreaVMarker, state);
}

void ScrollArea::drawHMarker(gcn::Graphics *graphics)
{
    WidgetState state(getHorizontalMarkerDimension());
    if (mHasMouse && (mY > (getHeight() - getScrollbarWidth())))
        state.flags |= STATE_HOVERED;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), SkinType::ScrollAreaHMarker, state);
}

void ScrollArea::drawButton(gcn::Graphics *graphics,
                            SkinType skinType,
                            bool pressed,
                            const gcn::Rectangle &dim)
{
    WidgetState state(dim);
    if (pressed)
        state.flags |= STATE_SELECTED;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), skinType, state);
}

void ScrollArea::mouseMoved(gcn::MouseEvent& event)
{
    mX = event.getX();
    mY = event.getY();
}

void ScrollArea::mouseEntered(gcn::MouseEvent& event)
{
    mHasMouse = true;
}

void ScrollArea::mouseExited(gcn::MouseEvent& event)
{
    mHasMouse = false;
}
