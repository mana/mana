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


ScrollArea::ScrollArea():
    gcn::ScrollArea()
{
    setBorderSize(2);
}

ScrollArea::ScrollArea(gcn::Widget *widget):
    gcn::ScrollArea(widget)
{
    setBorderSize(2);
}

void ScrollArea::draw(gcn::Graphics *graphics)
{
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

    draw_skinned_rect(gui_bitmap, &gui_skin.textbox.bg, x, y, w, h);
}
