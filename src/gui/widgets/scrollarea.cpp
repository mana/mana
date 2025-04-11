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

#include <guichan/exception.hpp>

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

void ScrollArea::setShowButtons(bool showButtons)
{
    mShowButtons = showButtons;
}

void ScrollArea::init()
{
    // Draw background by default
    setOpaque(true);

    auto theme = gui->getTheme();

    int scrollBarWidth = theme->getSkin(SkinType::ScrollAreaVBar).width;
    if (scrollBarWidth > 0)
        setScrollbarWidth(scrollBarWidth);

    auto &scrollAreaSkin = theme->getSkin(SkinType::ScrollArea);
    setShowButtons(scrollAreaSkin.showButtons);

    if (auto content = getContent())
        content->setFrameSize(scrollAreaSkin.padding);

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

void ScrollArea::drawChildren(gcn::Graphics *graphics)
{
    auto g = static_cast<Graphics*>(graphics);
    g->pushClipRect(getChildrenArea());

    gcn::ScrollArea::drawChildren(graphics);

    g->popClipRect();
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

static void drawButton(gcn::Graphics *graphics,
                       SkinType skinType,
                       bool pressed,
                       const gcn::Rectangle &dim)
{
    WidgetState state(dim);
    if (pressed)
        state.flags |= STATE_SELECTED;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), skinType, state);
}

void ScrollArea::drawUpButton(gcn::Graphics *graphics)
{
    if (!mShowButtons)
        return;

    drawButton(graphics, SkinType::ButtonUp, mUpButtonPressed, getUpButtonDimension());
}

void ScrollArea::drawDownButton(gcn::Graphics *graphics)
{
    if (!mShowButtons)
        return;

    drawButton(graphics, SkinType::ButtonDown, mDownButtonPressed, getDownButtonDimension());
}

void ScrollArea::drawLeftButton(gcn::Graphics *graphics)
{
    if (!mShowButtons)
        return;

    drawButton(graphics, SkinType::ButtonLeft, mLeftButtonPressed, getLeftButtonDimension());
}

void ScrollArea::drawRightButton(gcn::Graphics *graphics)
{
    if (!mShowButtons)
        return;

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
    if (state.height == 0)
        return;

    if (mHasMouse && (mX > (getWidth() - getScrollbarWidth())))
        state.flags |= STATE_HOVERED;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), SkinType::ScrollAreaVMarker, state);
}

void ScrollArea::drawHMarker(gcn::Graphics *graphics)
{
    WidgetState state(getHorizontalMarkerDimension());
    if (state.width == 0)
        return;

    if (mHasMouse && (mY > (getHeight() - getScrollbarWidth())))
        state.flags |= STATE_HOVERED;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), SkinType::ScrollAreaHMarker, state);
}

/**
 * Code copied from gcn::ScrollArea::checkPolicies to make sure it takes the
 * frame size of the content into account.
 */
void ScrollArea::checkPolicies()
{
    int w = getWidth();
    int h = getHeight();

    mHBarVisible = false;
    mVBarVisible = false;

    if (!getContent())
    {
        mHBarVisible = (mHPolicy == SHOW_ALWAYS);
        mVBarVisible = (mVPolicy == SHOW_ALWAYS);
        return;
    }

    const int contentFrameSize = getContent()->getFrameSize();
    w -= 2 * contentFrameSize;
    h -= 2 * contentFrameSize;

    if (mHPolicy == SHOW_AUTO &&
        mVPolicy == SHOW_AUTO)
    {
        if (getContent()->getWidth() <= w
            && getContent()->getHeight() <= h)
        {
            mHBarVisible = false;
            mVBarVisible = false;
        }

        if (getContent()->getWidth() > w)
        {
            mHBarVisible = true;
        }

        if ((getContent()->getHeight() > h)
            || (mHBarVisible && getContent()->getHeight() > h - mScrollbarWidth))
        {
            mVBarVisible = true;
        }

        if (mVBarVisible && getContent()->getWidth() > w - mScrollbarWidth)
        {
            mHBarVisible = true;
        }

        return;
    }

    switch (mHPolicy)
    {
    case SHOW_NEVER:
        mHBarVisible = false;
        break;

    case SHOW_ALWAYS:
        mHBarVisible = true;
        break;

    case SHOW_AUTO:
        if (mVPolicy == SHOW_NEVER)
        {
            mHBarVisible = getContent()->getWidth() > w;
        }
        else // (mVPolicy == SHOW_ALWAYS)
        {
            mHBarVisible = getContent()->getWidth() > w - mScrollbarWidth;
        }
        break;

    default:
        throw GCN_EXCEPTION("Horizontal scroll policy invalid.");
    }

    switch (mVPolicy)
    {
    case SHOW_NEVER:
        mVBarVisible = false;
        break;

    case SHOW_ALWAYS:
        mVBarVisible = true;
        break;

    case SHOW_AUTO:
        if (mHPolicy == SHOW_NEVER)
        {
            mVBarVisible = getContent()->getHeight() > h;
        }
        else // (mHPolicy == SHOW_ALWAYS)
        {
            mVBarVisible = getContent()->getHeight() > h - mScrollbarWidth;
        }
        break;
    default:
        throw GCN_EXCEPTION("Vertical scroll policy invalid.");
    }
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

/**
 * Code copied from gcn::ScrollArea::mousePressed to make it call our custom
 * getVerticalMarkerDimension and getHorizontalMarkerDimension functions.
 */
void ScrollArea::mousePressed(gcn::MouseEvent &mouseEvent)
{
    int x = mouseEvent.getX();
    int y = mouseEvent.getY();

    if (getUpButtonDimension().isPointInRect(x, y))
    {
        setVerticalScrollAmount(getVerticalScrollAmount()
                                - mUpButtonScrollAmount);
        mUpButtonPressed = true;
    }
    else if (getDownButtonDimension().isPointInRect(x, y))
    {
        setVerticalScrollAmount(getVerticalScrollAmount()
                                + mDownButtonScrollAmount);
        mDownButtonPressed = true;
    }
    else if (getLeftButtonDimension().isPointInRect(x, y))
    {
        setHorizontalScrollAmount(getHorizontalScrollAmount()
                                  - mLeftButtonScrollAmount);
        mLeftButtonPressed = true;
    }
    else if (getRightButtonDimension().isPointInRect(x, y))
    {
        setHorizontalScrollAmount(getHorizontalScrollAmount()
                                  + mRightButtonScrollAmount);
        mRightButtonPressed = true;
    }
    else if (getVerticalMarkerDimension().isPointInRect(x, y))
    {
        mIsHorizontalMarkerDragged = false;
        mIsVerticalMarkerDragged = true;

        mVerticalMarkerDragOffset = y - getVerticalMarkerDimension().y;
    }
    else if (getVerticalBarDimension().isPointInRect(x,y))
    {
        if (y < getVerticalMarkerDimension().y)
        {
            setVerticalScrollAmount(getVerticalScrollAmount()
                                    - (int)(getChildrenArea().height * 0.95));
        }
        else
        {
            setVerticalScrollAmount(getVerticalScrollAmount()
                                    + (int)(getChildrenArea().height * 0.95));
        }
    }
    else if (getHorizontalMarkerDimension().isPointInRect(x, y))
    {
        mIsHorizontalMarkerDragged = true;
        mIsVerticalMarkerDragged = false;

        mHorizontalMarkerDragOffset = x - getHorizontalMarkerDimension().x;
    }
    else if (getHorizontalBarDimension().isPointInRect(x,y))
    {
        if (x < getHorizontalMarkerDimension().x)
        {
            setHorizontalScrollAmount(getHorizontalScrollAmount()
                                      - (int)(getChildrenArea().width * 0.95));
        }
        else
        {
            setHorizontalScrollAmount(getHorizontalScrollAmount()
                                      + (int)(getChildrenArea().width * 0.95));
        }
    }
}

/**
 * Code copied from gcn::ScrollArea::mouseDragged to make it call our custom
 * getVerticalMarkerDimension and getHorizontalMarkerDimension functions.
 */
void ScrollArea::mouseDragged(gcn::MouseEvent &mouseEvent)
{
    if (mIsVerticalMarkerDragged)
    {
        int pos = mouseEvent.getY() - getVerticalBarDimension().y  - mVerticalMarkerDragOffset;
        int length = getVerticalMarkerDimension().height;

        gcn::Rectangle barDim = getVerticalBarDimension();

        if ((barDim.height - length) > 0)
        {
            setVerticalScrollAmount((getVerticalMaxScroll() * pos)
                                    / (barDim.height - length));
        }
        else
        {
            setVerticalScrollAmount(0);
        }
    }

    if (mIsHorizontalMarkerDragged)
    {
        int pos = mouseEvent.getX() - getHorizontalBarDimension().x  - mHorizontalMarkerDragOffset;
        int length = getHorizontalMarkerDimension().width;

        gcn::Rectangle barDim = getHorizontalBarDimension();

        if ((barDim.width - length) > 0)
        {
            setHorizontalScrollAmount((getHorizontalMaxScroll() * pos)
                                      / (barDim.width - length));
        }
        else
        {
            setHorizontalScrollAmount(0);
        }
    }

    mouseEvent.consume();
}

gcn::Rectangle ScrollArea::getUpButtonDimension()
{
    if (!mVBarVisible || !mShowButtons)
        return gcn::Rectangle();

    return gcn::Rectangle(getWidth() - mScrollbarWidth, 0, mScrollbarWidth, mScrollbarWidth);
}

gcn::Rectangle ScrollArea::getDownButtonDimension()
{
    if (!mVBarVisible || !mShowButtons)
        return gcn::Rectangle();

    gcn::Rectangle dim(getWidth() - mScrollbarWidth,
                       getHeight() - mScrollbarWidth,
                       mScrollbarWidth,
                       mScrollbarWidth);

    if (mHBarVisible)
        dim.y -= mScrollbarWidth;

    return dim;
}

gcn::Rectangle ScrollArea::getLeftButtonDimension()
{
    if (!mHBarVisible || !mShowButtons)
        return gcn::Rectangle();

    return gcn::Rectangle(0, getHeight() - mScrollbarWidth, mScrollbarWidth, mScrollbarWidth);
}

gcn::Rectangle ScrollArea::getRightButtonDimension()
{
    if (!mHBarVisible || !mShowButtons)
        return gcn::Rectangle();

    gcn::Rectangle dim(getWidth() - mScrollbarWidth,
                       getHeight() - mScrollbarWidth,
                       mScrollbarWidth,
                       mScrollbarWidth);

    if (mVBarVisible)
        dim.x -= mScrollbarWidth;

    return dim;
}

gcn::Rectangle ScrollArea::getVerticalBarDimension()
{
    if (!mVBarVisible)
        return gcn::Rectangle();

    gcn::Rectangle dim(getWidth() - mScrollbarWidth,
                       getUpButtonDimension().height,
                       mScrollbarWidth,
                       getHeight()
                           - getUpButtonDimension().height
                           - getDownButtonDimension().height);

    if (mHBarVisible)
        dim.height -= mScrollbarWidth;

    if (dim.height < 0)
        dim.height = 0;

    return dim;
}

gcn::Rectangle ScrollArea::getHorizontalBarDimension()
{
    if (!mHBarVisible)
        return gcn::Rectangle();

    gcn::Rectangle dim(getLeftButtonDimension().width,
                       getHeight() - mScrollbarWidth,
                       getWidth()
                           - getLeftButtonDimension().width
                           - getRightButtonDimension().width,
                       mScrollbarWidth);

    if (mVBarVisible)
        dim.width -= mScrollbarWidth;

    if (dim.width < 0)
        dim.width = 0;

    return dim;
}

static void getMarkerValues(int barSize,
                            int maxScroll, int scrollAmount,
                            int contentHeight, int viewHeight,
                            int fixedMarkerSize, int minMarkerSize,
                            int &markerSize, int &markerPos)
{
    if (fixedMarkerSize == 0)
    {
        if (contentHeight != 0 && contentHeight > viewHeight)
            markerSize = std::max((barSize * viewHeight) / contentHeight, minMarkerSize);
        else
            markerSize = barSize;
    }
    else
    {
        if (contentHeight > viewHeight)
            markerSize = fixedMarkerSize;
        else
            markerSize = 0;
    }

    // Hide the marker when it doesn't fit
    if (markerSize > barSize)
        markerSize = 0;

    if (maxScroll != 0)
        markerPos = ((barSize - markerSize) * scrollAmount + maxScroll / 2) / maxScroll;
    else
        markerPos = 0;
}

gcn::Rectangle ScrollArea::getVerticalMarkerDimension()
{
    if (!mVBarVisible)
        return gcn::Rectangle();

    auto &markerSkin = gui->getTheme()->getSkin(SkinType::ScrollAreaVMarker);
    const gcn::Rectangle barDim = getVerticalBarDimension();

    int contentHeight = 0;
    if (auto content = getContent())
        contentHeight = content->getHeight() + content->getFrameSize() * 2;

    int length;
    int pos;

    getMarkerValues(barDim.height,
                    getVerticalMaxScroll(),
                    getVerticalScrollAmount(),
                    contentHeight,
                    getChildrenArea().height,
                    markerSkin.height,
                    mScrollbarWidth,
                    length,
                    pos);

    return gcn::Rectangle(barDim.x, barDim.y + pos, mScrollbarWidth, length);
}

gcn::Rectangle ScrollArea::getHorizontalMarkerDimension()
{
    if (!mHBarVisible)
        return gcn::Rectangle();

    auto &markerSkin = gui->getTheme()->getSkin(SkinType::ScrollAreaHMarker);
    const gcn::Rectangle barDim = getHorizontalBarDimension();

    int contentWidth = 0;
    if (auto content = getContent())
        contentWidth = content->getWidth() + content->getFrameSize() * 2;

    int length;
    int pos;

    getMarkerValues(barDim.width,
                    getHorizontalMaxScroll(),
                    getHorizontalScrollAmount(),
                    contentWidth,
                    getChildrenArea().width,
                    markerSkin.width,
                    mScrollbarWidth,
                    length,
                    pos);

    return gcn::Rectangle(barDim.x + pos, barDim.y, length, mScrollbarWidth);
}
