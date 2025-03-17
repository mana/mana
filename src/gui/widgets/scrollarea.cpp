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
    addWidgetListener(this);
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

    setUpButtonScrollAmount(2);
    setDownButtonScrollAmount(2);
    setLeftButtonScrollAmount(2);
    setRightButtonScrollAmount(2);
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
    setFrameSize(mOpaque ? 2 : 0);
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
    graphics->setColor(gcn::Color(0, 0, 0, 32));
    graphics->fillRectangle(getVerticalBarDimension());
    graphics->setColor(gcn::Color(255, 255, 255));
}

void ScrollArea::drawHBar(gcn::Graphics *graphics)
{
    graphics->setColor(gcn::Color(0, 0, 0, 32));
    graphics->fillRectangle(getHorizontalBarDimension());
    graphics->setColor(gcn::Color(255, 255, 255));
}

void ScrollArea::drawVMarker(gcn::Graphics *graphics)
{
    drawMarker(static_cast<Graphics *>(graphics),
               mHasMouse && (mX > (getWidth() - getScrollbarWidth())),
               getVerticalMarkerDimension());
}

void ScrollArea::drawHMarker(gcn::Graphics *graphics)
{
    drawMarker(static_cast<Graphics *>(graphics),
               mHasMouse && (mY > (getHeight() - getScrollbarWidth())),
               getHorizontalMarkerDimension());
}

void ScrollArea::drawButton(gcn::Graphics *graphics,
                            SkinType skinType,
                            bool pressed,
                            const gcn::Rectangle &dim)
{
    WidgetState state;
    state.x = dim.x;
    state.y = dim.y;
    state.width = dim.width;
    state.height = dim.height;
    if (pressed)
        state.flags |= STATE_SELECTED;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), skinType, state);
}

void ScrollArea::drawMarker(gcn::Graphics *graphics, bool hovered, const gcn::Rectangle &dim)
{
    WidgetState state;
    state.x = dim.x;
    state.y = dim.y;
    state.width = dim.width;
    state.height = dim.height;
    if (hovered)
        state.flags |= STATE_HOVERED;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), SkinType::ScrollBar, state);
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

void ScrollArea::widgetResized(const gcn::Event &event)
{
    if (auto content = getContent())
    {
        content->setSize(getWidth() - 2 * getFrameSize(),
                         getHeight() - 2 * getFrameSize());
    }
}
