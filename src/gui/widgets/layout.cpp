/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
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

#include "layout.h"

void Layout::resizeGrid(int w, int h)
{
    bool extW = w && w > (int)mColWidths.size(),
         extH = h && h > (int)mRowHeights.size();
    if (!extW && !extH) return;

    if (extH)
    {
        mRowHeights.resize(h, -1);
        mCells.resize(h);
        if (!extW) w = (int)mColWidths.size();
    }

    mColWidths.resize(w, -1);
    for (std::vector< std::vector< Cell > >::iterator
         i = mCells.begin(), i_end = mCells.end(); i != i_end; ++i)
    {
        i->resize(w);
    }
}

void Layout::setColWidth(int n, int w)
{
    resizeGrid(n + 1, 0);
    mColWidths[n] = w;
}

void Layout::setRowHeight(int n, int h)
{
    resizeGrid(0, n + 1);
    mRowHeights[n] = h;
}

Cell &Layout::place(gcn::Widget *widget, int x, int y, int w, int h)
{
    resizeGrid(x + w, y + h);
    Cell &cell = mCells[y][x];
    cell.mWidget = widget;
    cell.mColExtent = w;
    cell.mRowExtent = h;
    cell.mPadding = 0;
    cell.mHAlign = Cell::FILL;
    cell.mVAlign = Cell::FILL;
    return cell;
}

static void align(int &pos, int &cur, int upp, Cell::Alignment a)
{
    cur = std::min(cur, upp);
    switch (a)
    {
        case Cell::LEFT:
            return;
        case Cell::RIGHT:
            pos += upp - cur;
            return;
        case Cell::CENTER:
            pos += (upp - cur) / 2;
            return;
        case Cell::FILL:
            cur = upp;
            return;
    }
}

void Layout::reflow()
{
    int gridW = mColWidths.size(), gridH = mRowHeights.size();

    std::vector< int > widths(gridW, 0);
    std::vector< int > heights(gridH, 0);

    for (int gridY = 0; gridY < gridH; ++gridY)
    {
        for (int gridX = 0; gridX < gridW; ++gridX)
        {
            Cell &cell = mCells[gridY][gridX];
            if (!cell.mWidget) continue;

            if (cell.mColExtent == 1)
            {
                int w = cell.mWidget->getWidth() + cell.mPadding * 2;
                if (w > widths[gridX]) widths[gridX] = w;
            }

            if (cell.mRowExtent == 1)
            {
                int h = cell.mWidget->getHeight() + cell.mPadding * 2;
                if (h > heights[gridY]) heights[gridY] = h;
            }
        }
    }

    for (int gridX = 0; gridX < gridW; ++gridX)
    {
        int w = mColWidths[gridX];
        if (w != -1) widths[gridX] = w;
    }

    for (int gridY = 0; gridY < gridH; ++gridY)
    {
        int h = mRowHeights[gridY];
        if (h != -1) heights[gridY] = h;
    }

    int y = 0;
    for (int gridY = 0; gridY < gridH; ++gridY)
    {
        int h = heights[gridY];
        int x = 0;
        for (int gridX = 0; gridX < gridW; ++gridX)
        {
            int w = widths[gridX];
            Cell &cell = mCells[gridY][gridX];
            if (cell.mWidget)
            {
                int ew = w - cell.mPadding * 2,
                    eh = h - cell.mPadding * 2;
                for (int i = 1; i < cell.mColExtent; ++i)
                    ew += widths[gridX + i] + mPadding;
                for (int i = 1; i < cell.mRowExtent; ++i)
                    eh += heights[gridY + i] + mPadding;
                int dw = cell.mWidget->getWidth(),
                    dh = cell.mWidget->getHeight();
                int dx = x + cell.mPadding, dy = y + cell.mPadding;
                align(dx, dw, ew, cell.mHAlign);
                align(dy, dh, eh, cell.mVAlign);
                cell.mWidget->setDimension(gcn::Rectangle(dx, dy, dw, dh));
            }
            x += w + mPadding;
        }
        y += h + mPadding;
    }
}
