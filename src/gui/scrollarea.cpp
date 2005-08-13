/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
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

#include "scrollarea.h"

#include "../graphics.h"

#include "../graphic/imagerect.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

int ScrollArea::instances = 0;
ImageRect ScrollArea::background;
ImageRect ScrollArea::vMarker;
Image *ScrollArea::buttons[4][2];

ScrollArea::ScrollArea():
    gcn::ScrollArea()
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
    instances--;

    if (instances == 0)
    {
        for (int a = 0; a < 9; a++) {
            delete background.grid[a];
            delete vMarker.grid[a];
        }

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

    if (instances == 0)
    {
        // Load the background skin
        ResourceManager *resman = ResourceManager::getInstance();
        Image *textbox = resman->getImage("graphics/gui/deepbox.png");
        int bggridx[4] = {0, 3, 28, 31};
        int bggridy[4] = {0, 3, 28, 31};
        int a = 0, x, y;

        for (y = 0; y < 3; y++) {
            for (x = 0; x < 3; x++) {
                background.grid[a] = textbox->getSubImage(
                        bggridx[x], bggridy[y],
                        bggridx[x + 1] - bggridx[x] + 1,
                        bggridy[y + 1] - bggridy[y] + 1);
                a++;
            }
        }

        textbox->decRef();

        // Load vertical scrollbar skin
        Image *vscroll = resman->getImage("graphics/gui/vscroll_grey.png");
        int vsgridx[4] = {0, 4, 7, 11};
        int vsgridy[4] = {0, 4, 15, 19};
        a = 0;

        for (y = 0; y < 3; y++) {
            for (x = 0; x < 3; x++) {
                vMarker.grid[a] = vscroll->getSubImage(
                        vsgridx[x], vsgridy[y],
                        vsgridx[x + 1] - vsgridx[x],
                        vsgridy[y + 1] - vsgridy[y]);
                a++;
            }
        }

        vscroll->decRef();

        buttons[UP][0] =
            resman->getImage("graphics/gui/vscroll_up_default.png");
        buttons[DOWN][0] =
            resman->getImage("graphics/gui/vscroll_down_default.png");
        buttons[LEFT][0] =
            resman->getImage("graphics/gui/hscroll_left_default.png");
        buttons[RIGHT][0] =
            resman->getImage("graphics/gui/hscroll_right_default.png");
        buttons[UP][1] =
            resman->getImage("graphics/gui/vscroll_up_pressed.png");
        buttons[DOWN][1] =
            resman->getImage("graphics/gui/vscroll_down_pressed.png");
        buttons[LEFT][1] =
            resman->getImage("graphics/gui/hscroll_left_pressed.png");
        buttons[RIGHT][1] =
            resman->getImage("graphics/gui/hscroll_right_pressed.png");
    }

    instances++;
}

void ScrollArea::logic()
{
    gcn::ScrollArea::logic();

    // When no scrollbar in a certain direction, adapt content size to match
    // the content dimension exactly.
    if (mContent != NULL)
    {
        if (getHorizontalScrollPolicy() == gcn::ScrollArea::SHOW_NEVER)
        {
            mContent->setWidth(getContentDimension().width -
                    2 * mContent->getBorderSize());
        }
        if (getVerticalScrollPolicy() == gcn::ScrollArea::SHOW_NEVER)
        {
            mContent->setHeight(getContentDimension().height -
                    2 * mContent->getBorderSize());
        }
    }
}

void ScrollArea::draw(gcn::Graphics *graphics)
{
    checkPolicies();

    int alpha = getBaseColor().a;
    gcn::Color highlightColor = getBaseColor() + 0x303030;
    highlightColor.a = alpha;
    gcn::Color shadowColor = getBaseColor() - 0x303030;
    shadowColor.a = alpha;

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

    if (mContent)
    {
        gcn::Rectangle contdim = mContent->getDimension();
        graphics->pushClipArea(getContentDimension());

        if (mContent->getBorderSize() > 0)
        {
            gcn::Rectangle rec = mContent->getDimension();
            rec.x -= mContent->getBorderSize();
            rec.y -= mContent->getBorderSize();
            rec.width += 2 * mContent->getBorderSize();
            rec.height += 2 * mContent->getBorderSize();
            graphics->pushClipArea(rec);
            mContent->drawBorder(graphics);
            graphics->popClipArea();
        }

        graphics->pushClipArea(contdim);
        mContent->draw(graphics);
        graphics->popClipArea();
        graphics->popClipArea();
    }
}

void ScrollArea::drawBorder(gcn::Graphics *graphics)
{
    int x, y, w, h, bs;
    getAbsolutePosition(x, y);
    bs = getBorderSize();
    w = getWidth() + bs * 2;
    h = getHeight() + bs * 2;
    x -= bs;
    y -= bs;

    if (isOpaque()) {
        dynamic_cast<Graphics*>(graphics)->drawImageRect(x, y, w, h, background);
    }
}

void ScrollArea::setOpaque(bool opaque)
{
    this->opaque = opaque;

    if (opaque) {
        setBorderSize(2);
    }
    else {
        setBorderSize(0);
    }
}

bool ScrollArea::isOpaque()
{
    return opaque;
}

void ScrollArea::drawButton(gcn::Graphics *graphics, BUTTON_DIR dir)
{
    int x, y, state = 0;
    gcn::Rectangle dim;

    getAbsolutePosition(x,y);

    switch(dir) {
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

    dynamic_cast<Graphics*>(graphics)->drawImage(
            buttons[dir][state], x + dim.x, y + dim.y);
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
    gcn::Rectangle dim = getVerticalBarDimension();
    graphics->setColor(gcn::Color(0, 0, 0, 32));
    graphics->fillRectangle(dim);
    graphics->setColor(gcn::Color(255, 255, 255));
}

void ScrollArea::drawHBar(gcn::Graphics *graphics)
{
    gcn::Rectangle dim = getHorizontalBarDimension();
    graphics->setColor(gcn::Color(0, 0, 0, 32));
    graphics->fillRectangle(dim);
    graphics->setColor(gcn::Color(255, 255, 255));
}

void ScrollArea::drawVMarker(gcn::Graphics *graphics)
{
    int x, y;
    gcn::Rectangle dim = getVerticalMarkerDimension();
    getAbsolutePosition(x, y);

    dynamic_cast<Graphics*>(graphics)->drawImageRect(
            x + dim.x, y + dim.y, dim.width, dim.height, vMarker);
}

void ScrollArea::drawHMarker(gcn::Graphics *graphics)
{
    int x, y;
    gcn::Rectangle dim = getHorizontalMarkerDimension();
    getAbsolutePosition(x, y);

    dynamic_cast<Graphics*>(graphics)->drawImageRect(
            x + dim.x, y + dim.y, dim.width, dim.height, vMarker);
}
