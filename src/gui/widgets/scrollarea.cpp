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

#include "configuration.h"
#include "graphics.h"

#include "resources/image.h"
#include "resources/theme.h"

#include "utils/dtor.h"

int ScrollArea::instances = 0;
float ScrollArea::mAlpha = 1.0;
ImageRect ScrollArea::background;
ImageRect ScrollArea::vMarker;
ImageRect ScrollArea::vMarkerHi;
Image *ScrollArea::buttons[4][2];

ScrollArea::ScrollArea():
    gcn::ScrollArea(),
    mX(0),
    mY(0),
    mHasMouse(false),
    mOpaque(true)
{
    addWidgetListener(this);
    init();
}

ScrollArea::ScrollArea(gcn::Widget *widget):
    gcn::ScrollArea(widget),
    mX(0),
    mY(0),
    mHasMouse(false),
    mOpaque(true)
{
    init();
}

ScrollArea::~ScrollArea()
{
    // Garbage collection
    delete getContent();

    instances--;

    if (instances == 0)
    {
        std::for_each(background.grid, background.grid + 9, dtor<Image*>());
        std::for_each(vMarker.grid, vMarker.grid + 9, dtor<Image*>());
        std::for_each(vMarkerHi.grid, vMarkerHi.grid + 9, dtor<Image*>());

        buttons[UP][0]->decRef();
        buttons[UP][1]->decRef();
        buttons[DOWN][0]->decRef();
        buttons[DOWN][1]->decRef();
        buttons[LEFT][0]->decRef();
        buttons[LEFT][1]->decRef();
        buttons[RIGHT][0]->decRef();
        buttons[RIGHT][1]->decRef();
    }
}

void ScrollArea::init()
{
    // Draw background by default
    setOpaque(true);

    setUpButtonScrollAmount(2);
    setDownButtonScrollAmount(2);
    setLeftButtonScrollAmount(2);
    setRightButtonScrollAmount(2);

    if (instances == 0)
    {
        // Load the background skin
        Image *textbox = Theme::getImageFromTheme("deepbox.png");
        const int bggridx[4] = {0, 3, 28, 31};
        const int bggridy[4] = {0, 3, 28, 31};
        int a = 0, x, y;

        for (y = 0; y < 3; y++)
        {
            for (x = 0; x < 3; x++)
            {
                background.grid[a] = textbox->getSubImage(
                        bggridx[x], bggridy[y],
                        bggridx[x + 1] - bggridx[x] + 1,
                        bggridy[y + 1] - bggridy[y] + 1);
                background.grid[a]->setAlpha(config.getFloatValue("guialpha"));
                a++;
            }
        }

        textbox->decRef();

        // Load vertical scrollbar skin
        Image *vscroll = Theme::getImageFromTheme("vscroll_grey.png");
        Image *vscrollHi = Theme::getImageFromTheme("vscroll_highlight.png");

        int vsgridx[4] = {0, 4, 7, 11};
        int vsgridy[4] = {0, 4, 15, 19};
        a = 0;

        for (y = 0; y < 3; y++)
        {
            for (x = 0; x < 3; x++)
            {
                vMarker.grid[a] = vscroll->getSubImage(
                        vsgridx[x], vsgridy[y],
                        vsgridx[x + 1] - vsgridx[x],
                        vsgridy[y + 1] - vsgridy[y]);
                vMarkerHi.grid[a] = vscrollHi->getSubImage(
                        vsgridx[x], vsgridy[y],
                        vsgridx[x + 1] - vsgridx[x],
                        vsgridy[y + 1] - vsgridy[y]);
                vMarker.grid[a]->setAlpha(config.getFloatValue("guialpha"));
                vMarkerHi.grid[a]->setAlpha(config.getFloatValue("guialpha"));
                a++;
            }
        }

        vscroll->decRef();
        vscrollHi->decRef();

        buttons[UP][0] =
            Theme::getImageFromTheme("vscroll_up_default.png");
        buttons[DOWN][0] =
            Theme::getImageFromTheme("vscroll_down_default.png");
        buttons[LEFT][0] =
            Theme::getImageFromTheme("hscroll_left_default.png");
        buttons[RIGHT][0] =
            Theme::getImageFromTheme("hscroll_right_default.png");
        buttons[UP][1] =
            Theme::getImageFromTheme("vscroll_up_pressed.png");
        buttons[DOWN][1] =
            Theme::getImageFromTheme("vscroll_down_pressed.png");
        buttons[LEFT][1] =
            Theme::getImageFromTheme("hscroll_left_pressed.png");
        buttons[RIGHT][1] =
            Theme::getImageFromTheme("hscroll_right_pressed.png");
    }

    instances++;
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

void ScrollArea::updateAlpha()
{
    float alpha = std::max(config.getFloatValue("guialpha"),
                               Theme::instance()->getMinimumOpacity());

    if (alpha != mAlpha)
    {
        mAlpha = alpha;
        for (int a = 0; a < 9; a++)
        {
            background.grid[a]->setAlpha(mAlpha);
            vMarker.grid[a]->setAlpha(mAlpha);
            vMarkerHi.grid[a]->setAlpha(mAlpha);
        }
    }
}

void ScrollArea::draw(gcn::Graphics *graphics)
{
    if (mVBarVisible)
    {
        drawUpButton(graphics);
        drawDownButton(graphics);
        drawVBar(graphics);
        drawVMarker(graphics);
    }

    if (mHBarVisible)
    {
        drawLeftButton(graphics);
        drawRightButton(graphics);
        drawHBar(graphics);
        drawHMarker(graphics);
    }

    if (mHBarVisible && mVBarVisible)
    {
        graphics->setColor(getBaseColor());
        graphics->fillRectangle(gcn::Rectangle(getWidth() - mScrollbarWidth,
                    getHeight() - mScrollbarWidth,
                    mScrollbarWidth,
                    mScrollbarWidth));
    }

    updateAlpha();

    drawChildren(graphics);
}

void ScrollArea::drawFrame(gcn::Graphics *graphics)
{
    if (mOpaque)
    {
        const int bs = getFrameSize();
        const int w = getWidth() + bs * 2;
        const int h = getHeight() + bs * 2;

        static_cast<Graphics*>(graphics)->
                drawImageRect(0, 0, w, h, background);
    }
}

void ScrollArea::setOpaque(bool opaque)
{
    mOpaque = opaque;
    setFrameSize(mOpaque ? 2 : 0);
}

void ScrollArea::drawButton(gcn::Graphics *graphics, BUTTON_DIR dir)
{
    int state = 0;
    gcn::Rectangle dim;

    switch (dir)
    {
        case UP:
            state = mUpButtonPressed ? 1 : 0;
            dim = getUpButtonDimension();
            break;
        case DOWN:
            state = mDownButtonPressed ? 1 : 0;
            dim = getDownButtonDimension();
            break;
        case LEFT:
            state = mLeftButtonPressed ? 1 : 0;
            dim = getLeftButtonDimension();
            break;
        case RIGHT:
            state = mRightButtonPressed ? 1 : 0;
            dim = getRightButtonDimension();
            break;
    }

    static_cast<Graphics*>(graphics)->
        drawImage(buttons[dir][state], dim.x, dim.y);
}

void ScrollArea::drawUpButton(gcn::Graphics *graphics)
{
    drawButton(graphics, UP);
}

void ScrollArea::drawDownButton(gcn::Graphics *graphics)
{
    drawButton(graphics, DOWN);
}

void ScrollArea::drawLeftButton(gcn::Graphics *graphics)
{
    drawButton(graphics, LEFT);
}

void ScrollArea::drawRightButton(gcn::Graphics *graphics)
{
    drawButton(graphics, RIGHT);
}

void ScrollArea::drawVBar(gcn::Graphics *graphics)
{
    const gcn::Rectangle dim = getVerticalBarDimension();
    graphics->setColor(gcn::Color(0, 0, 0, 32));
    graphics->fillRectangle(dim);
    graphics->setColor(gcn::Color(255, 255, 255));
}

void ScrollArea::drawHBar(gcn::Graphics *graphics)
{
    const gcn::Rectangle dim = getHorizontalBarDimension();
    graphics->setColor(gcn::Color(0, 0, 0, 32));
    graphics->fillRectangle(dim);
    graphics->setColor(gcn::Color(255, 255, 255));
}

void ScrollArea::drawVMarker(gcn::Graphics *graphics)
{
    gcn::Rectangle dim = getVerticalMarkerDimension();

    if ((mHasMouse) && (mX > (getWidth() - getScrollbarWidth())))
        static_cast<Graphics*>(graphics)->
            drawImageRect(dim.x, dim.y, dim.width, dim.height, vMarkerHi);
    else
        static_cast<Graphics*>(graphics)->
            drawImageRect(dim.x, dim.y, dim.width, dim.height,vMarker);
}

void ScrollArea::drawHMarker(gcn::Graphics *graphics)
{
    gcn::Rectangle dim = getHorizontalMarkerDimension();

    if ((mHasMouse) && (mY > (getHeight() - getScrollbarWidth())))
        static_cast<Graphics*>(graphics)->
            drawImageRect(dim.x, dim.y, dim.width, dim.height, vMarkerHi);
    else
        static_cast<Graphics*>(graphics)->
            drawImageRect(dim.x, dim.y, dim.width, dim.height, vMarker);
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
    getContent()->setSize(getWidth() - 2 * getFrameSize(), getHeight() - 2 * getFrameSize());
}
