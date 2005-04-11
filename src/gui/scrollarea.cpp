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
#include "gui.h"
#include "../resources/resourcemanager.h"
#include "../main.h"

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

void ScrollArea::init()
{
    setBorderSize(2);

    // Load the background skin
    ResourceManager *resman = ResourceManager::getInstance();
    Image *textbox = resman->getImage("graphics/gui/textbox.png");
    int bggridx[4] = {0, 9, 16, 25};
    int bggridy[4] = {0, 4, 19, 24};
    int a = 0, x, y;

    // Load GUI alpha setting
    guiAlpha = config.getValue("guialpha", 0.8f);

    // Set GUI alpha level
    textbox->setAlpha(guiAlpha);
    
    for (y = 0; y < 3; y++) {
        for (x = 0; x < 3; x++) {
            background.grid[a] = textbox->getSubImage(
                    bggridx[x], bggridy[y],
                    bggridx[x + 1] - bggridx[x] + 1,
                    bggridy[y + 1] - bggridy[y] + 1);
            a++;
        }
    }

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
    
    hscroll_left_default = resman->getImage("graphics/gui/hscroll_left_default.png");
    hscroll_right_default = resman->getImage("graphics/gui/hscroll_right_default.png");
    vscroll_down_default = resman->getImage("graphics/gui/vscroll_down_default.png");
    vscroll_up_default = resman->getImage("graphics/gui/vscroll_up_default.png");
    hscroll_left_pressed = resman->getImage("graphics/gui/hscroll_left_pressed.png");
    hscroll_right_pressed = resman->getImage("graphics/gui/hscroll_right_pressed.png");
    vscroll_down_pressed = resman->getImage("graphics/gui/vscroll_down_pressed.png");
    vscroll_up_pressed = resman->getImage("graphics/gui/vscroll_up_pressed.png");
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

    ((Graphics*)graphics)->drawImageRect(x, y, w, h, background);
}

void ScrollArea::drawUpButton(gcn::Graphics *graphics)
{
    gcn::Rectangle dim = getUpButtonDimension();
    int x, y;
    getAbsolutePosition(x, y);
    if (mUpButtonPressed)
        vscroll_up_pressed->draw(screen, x + dim.x, y + dim.y);
    else
        vscroll_up_default->draw(screen, x + dim.x, y + dim.y);
}

void ScrollArea::drawDownButton(gcn::Graphics *graphics)
{
    gcn::Rectangle dim = getDownButtonDimension();
    int x, y;
    getAbsolutePosition(x, y);
    if (mDownButtonPressed)
        vscroll_down_pressed->draw(screen, x + dim.x, y + dim.y);
    else
        vscroll_down_default->draw(screen, x + dim.x, y + dim.y);

}

void ScrollArea::drawLeftButton(gcn::Graphics *graphics)
{
    gcn::Rectangle dim = getLeftButtonDimension();
    int x, y;
    getAbsolutePosition(x, y);
    if (mLeftButtonPressed)
        hscroll_left_pressed->draw(screen, x + dim.x, y + dim.y);
    else
        hscroll_left_default->draw(screen, x + dim.x, y + dim.y);

}

void ScrollArea::drawRightButton(gcn::Graphics *graphics)
{
    gcn::Rectangle dim = getRightButtonDimension();
    int x, y;
    getAbsolutePosition(x, y);
    if (mRightButtonPressed)
        hscroll_right_pressed->draw(screen, x + dim.x, y + dim.y);
    else
        hscroll_right_default->draw(screen, x + dim.x, y + dim.y);

}

void ScrollArea::drawVBar(gcn::Graphics *graphics)
{
    int x, y;
    gcn::Rectangle dim = getVerticalBarDimension();
    getAbsolutePosition(x, y);

    ((Graphics*)graphics)->drawImageRect(
            x + dim.x, y + dim.y, dim.width, dim.height, background);
}

void ScrollArea::drawHBar(gcn::Graphics *graphics)
{
    int x, y;
    gcn::Rectangle dim = getHorizontalBarDimension();
    getAbsolutePosition(x, y);

    ((Graphics*)graphics)->drawImageRect(
            x + dim.x, y + dim.y, dim.width, dim.height, background);
}

void ScrollArea::drawVMarker(gcn::Graphics *graphics)
{
    int x, y;
    gcn::Rectangle dim = getVerticalMarkerDimension();
    getAbsolutePosition(x, y);

    ((Graphics*)graphics)->drawImageRect(
            x + dim.x, y + dim.y, dim.width, dim.height, vMarker);
}

void ScrollArea::drawHMarker(gcn::Graphics *graphics)
{
    int x, y;
    gcn::Rectangle dim = getHorizontalMarkerDimension();
    getAbsolutePosition(x, y);

    ((Graphics*)graphics)->drawImageRect(
            x + dim.x, y + dim.y, dim.width, dim.height, vMarker);
}
