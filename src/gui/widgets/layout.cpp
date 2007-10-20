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
    bool extW = w && w > (int)mSizes[0].size(),
         extH = h && h > (int)mSizes[1].size();
    if (!extW && !extH) return;

    if (extH)
    {
        mSizes[1].resize(h, 0);
        mCells.resize(h);
        if (!extW) w = mSizes[0].size();
    }

    if (extW)
    {
        mSizes[0].resize(w, 0);
    }

    for (std::vector< std::vector< Cell > >::iterator
         i = mCells.begin(), i_end = mCells.end(); i != i_end; ++i)
    {
        i->resize(w);
    }
}

void Layout::setColWidth(int n, int w)
{
    resizeGrid(n + 1, 0);
    mSizes[0][n] = w;
}

void Layout::setRowHeight(int n, int h)
{
    resizeGrid(0, n + 1);
    mSizes[1][n] = h;
}

void Layout::matchColWidth(int n1, int n2)
{
    resizeGrid(std::max(n1, n2) + 1, 0);
    std::vector< int > widths = compute(0, mW);
    int s = std::max(widths[n1], widths[n2]);
    mSizes[0][n1] = s;
    mSizes[0][n2] = s;
}

Cell &Layout::place(gcn::Widget *widget, int x, int y, int w, int h)
{
    resizeGrid(x + w, y + h);
    Cell &cell = mCells[y][x];
    cell.mWidget = widget;
    cell.mExtent[0] = w;
    cell.mExtent[1] = h;
    cell.mPadding = 0;
    cell.mAlign[0] = Cell::FILL;
    cell.mAlign[1] = Cell::FILL;
    return cell;
}

void Layout::align(int &pos, int &size, int dim, Cell &cell, int *sizes)
{
    int size_max = sizes[0] - cell.mPadding * 2;
    for (int i = 1; i < cell.mExtent[dim]; ++i)
        size_max += sizes[i] + mPadding;
    size = std::min(dim == 0 ? cell.mWidget->getWidth()
                             : cell.mWidget->getHeight(), size_max);
    pos += cell.mPadding;

    switch (cell.mAlign[dim])
    {
        case Cell::LEFT:
            return;
        case Cell::RIGHT:
            pos += size_max - size;
            return;
        case Cell::CENTER:
            pos += (size_max - size) / 2;
            return;
        case Cell::FILL:
            size = size_max;
            return;
    }
}

std::vector< int > Layout::compute(int dim, int upp)
{
    int gridW = mSizes[0].size(), gridH = mSizes[1].size();
    std::vector< int > sizes = mSizes[dim];

    // Compute minimum sizes.
    for (int gridY = 0; gridY < gridH; ++gridY)
    {
        for (int gridX = 0; gridX < gridW; ++gridX)
        {
            Cell &cell = mCells[gridY][gridX];
            if (!cell.mWidget) continue;

            if (cell.mExtent[dim] == 1)
            {
                int s = dim == 0 ? cell.mWidget->getWidth()
                                 : cell.mWidget->getHeight();
                int n = dim == 0 ? gridX : gridY;
                s += cell.mPadding * 2;
                if (s > sizes[n]) sizes[n] = s;
            }
        }
    }

    // Compute the FILL sizes.
    int nb = sizes.size();
    int nbFill = 0;
    for (int i = 0; i < nb; ++i)
    {
        if (mSizes[dim][i] == FILL) ++nbFill;
        if (sizes[i] > 0) upp -= sizes[i];
        upp -= mPadding;
    }
    upp += mPadding;

    if (upp <= 0 || nbFill == 0) return sizes;

    for (int i = 0; i < nb; ++i)
    {
        if (mSizes[dim][i] != FILL) continue;
        int s = (upp + nbFill / 2) / nbFill;
        sizes[i] += s;
        upp -= s;
        --nbFill;
    }

    return sizes;
}

void Layout::reflow()
{
    int gridW = mSizes[0].size(), gridH = mSizes[1].size();

    std::vector< int > widths = compute(0, mW);
    std::vector< int > heights = compute(1, mH);

    int y = mY;
    for (int gridY = 0; gridY < gridH; ++gridY)
    {
        int x = mX;
        for (int gridX = 0; gridX < gridW; ++gridX)
        {
            Cell &cell = mCells[gridY][gridX];
            if (cell.mWidget)
            {
                int dx = x, dy = y, dw, dh;
                align(dx, dw, 0, cell, &widths[gridX]);
                align(dy, dh, 1, cell, &heights[gridY]);
                cell.mWidget->setDimension(gcn::Rectangle(dx, dy, dw, dh));
            }
            x += widths[gridX] + mPadding;
            mNW = x - mX;
        }
        y += heights[gridY] + mPadding;
        mNH = y - mY;
    }
}

void Layout::flush()
{
    reflow();
    mY += mNH;
    mW = mNW - mPadding;
    mSizes[0].clear();
    mSizes[1].clear();
    mCells.clear();
}
