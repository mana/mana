/*
 *  The Mana World
 *  Copyright (C) 2007  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gui/widgets/layout.h"

#include <cassert>

ContainerPlacer ContainerPlacer::at(int x, int y)
{
    return ContainerPlacer(mContainer, &mCell->at(x, y));
}

LayoutCell &ContainerPlacer::operator()
    (int x, int y, gcn::Widget *wg, int w, int h)
{
    mContainer->add(wg);
    return mCell->place(wg, x, y, w, h);
}

LayoutCell::~LayoutCell()
{
    if (mType == ARRAY) delete mArray;
}

LayoutArray &LayoutCell::getArray()
{
    assert(mType != WIDGET);
    if (mType == ARRAY) return *mArray;
    mArray = new LayoutArray;
    mType = ARRAY;
    mExtent[0] = 1;
    mExtent[1] = 1;
    mPadding = 0;
    mAlign[0] = FILL;
    mAlign[1] = FILL;
    return *mArray;
}

void LayoutCell::reflow(int nx, int ny, int nw, int nh)
{
    assert(mType != NONE);
    nx += mPadding;
    ny += mPadding;
    nw -= 2 * mPadding;
    nh -= 2 * mPadding;
    if (mType == ARRAY)
        mArray->reflow(nx, ny, nw, nh);
    else
        mWidget->setDimension(gcn::Rectangle(nx, ny, nw, nh));
}

void LayoutCell::computeSizes()
{
    assert(mType == ARRAY);

    for (std::vector< std::vector< LayoutCell * > >::iterator
         i = mArray->mCells.begin(), i_end = mArray->mCells.end();
         i != i_end; ++i)
    {
        for (std::vector< LayoutCell * >::iterator
             j = i->begin(), j_end = i->end(); j != j_end; ++j)
        {
            LayoutCell *cell = *j;
            if (cell && cell->mType == ARRAY) cell->computeSizes();
        }
    }

    mSize[0] = mArray->getSize(0);
    mSize[1] = mArray->getSize(1);
}

LayoutArray::LayoutArray(): mSpacing(4)
{
}

LayoutArray::~LayoutArray()
{
    for (std::vector< std::vector< LayoutCell * > >::iterator
         i = mCells.begin(), i_end = mCells.end(); i != i_end; ++i)
    {
        for (std::vector< LayoutCell * >::iterator
             j = i->begin(), j_end = i->end(); j != j_end; ++j)
        {
            delete *j;
        }
    }
}

LayoutCell &LayoutArray::at(int x, int y, int w, int h)
{
    resizeGrid(x + w, y + h);
    LayoutCell *&cell = mCells[y][x];
    if (!cell)
    {
        cell = new LayoutCell;
    }
    return *cell;
}

void LayoutArray::resizeGrid(int w, int h)
{
    bool extW = w && w > (int)mSizes[0].size(),
         extH = h && h > (int)mSizes[1].size();
    if (!extW && !extH) return;

    if (extH)
    {
        mSizes[1].resize(h, Layout::AUTO_DEF);
        mCells.resize(h);
        if (!extW) w = mSizes[0].size();
    }

    if (extW)
    {
        mSizes[0].resize(w, Layout::AUTO_DEF);
    }

    for (std::vector< std::vector< LayoutCell * > >::iterator
         i = mCells.begin(), i_end = mCells.end(); i != i_end; ++i)
    {
        i->resize(w, NULL);
    }
}

void LayoutArray::setColWidth(int n, int w)
{
    resizeGrid(n + 1, 0);
    mSizes[0][n] = w;
}

void LayoutArray::setRowHeight(int n, int h)
{
    resizeGrid(0, n + 1);
    mSizes[1][n] = h;
}

void LayoutArray::matchColWidth(int n1, int n2)
{
    resizeGrid(std::max(n1, n2) + 1, 0);
    std::vector< short > widths = getSizes(0, Layout::AUTO_DEF);
    int s = std::max(widths[n1], widths[n2]);
    mSizes[0][n1] = s;
    mSizes[0][n2] = s;
}

void LayoutArray::extend(int x, int y, int w, int h)
{
    LayoutCell &cell = at(x, y, w, h);
    cell.mExtent[0] = w;
    cell.mExtent[1] = h;
}

LayoutCell &LayoutArray::place(gcn::Widget *widget, int x, int y, int w, int h)
{
    LayoutCell &cell = at(x, y, w, h);
    assert(cell.mType == LayoutCell::NONE);
    cell.mType = LayoutCell::WIDGET;
    cell.mWidget = widget;
    cell.mSize[0] = w == 1 ? widget->getWidth() : 0;
    cell.mSize[1] = h == 1 ? widget->getHeight() : 0;
    cell.mExtent[0] = w;
    cell.mExtent[1] = h;
    cell.mPadding = 0;
    cell.mAlign[0] = LayoutCell::FILL;
    cell.mAlign[1] = LayoutCell::FILL;
    short &cs = mSizes[0][x], &rs = mSizes[1][y];
    if (cs == Layout::AUTO_DEF && w == 1) cs = 0;
    if (rs == Layout::AUTO_DEF && h == 1) rs = 0;
    return cell;
}

void LayoutArray::align(int &pos, int &size, int dim,
                        LayoutCell const &cell, short *sizes) const
{
    int size_max = sizes[0];
    for (int i = 1; i < cell.mExtent[dim]; ++i)
        size_max += sizes[i] + mSpacing;
    size = std::min<int>(cell.mSize[dim], size_max);

    switch (cell.mAlign[dim])
    {
        case LayoutCell::LEFT:
            return;
        case LayoutCell::RIGHT:
            pos += size_max - size;
            return;
        case LayoutCell::CENTER:
            pos += (size_max - size) / 2;
            return;
        case LayoutCell::FILL:
            size = size_max;
            return;
    }
}

std::vector< short > LayoutArray::getSizes(int dim, int upp) const
{
    int gridW = mSizes[0].size(), gridH = mSizes[1].size();
    std::vector< short > sizes = mSizes[dim];

    // Compute minimum sizes.
    for (int gridY = 0; gridY < gridH; ++gridY)
    {
        for (int gridX = 0; gridX < gridW; ++gridX)
        {
            LayoutCell const *cell = mCells[gridY][gridX];
            if (!cell || cell->mType == LayoutCell::NONE) continue;

            if (cell->mExtent[dim] == 1)
            {
                int n = dim == 0 ? gridX : gridY;
                int s = cell->mSize[dim] + cell->mPadding * 2;
                if (s > sizes[n]) sizes[n] = s;
            }
        }
    }

    if (upp == Layout::AUTO_DEF) return sizes;

    // Compute the FILL sizes.
    int nb = sizes.size();
    int nbFill = 0;
    for (int i = 0; i < nb; ++i)
    {
        if (mSizes[dim][i] <= Layout::AUTO_DEF)
        {
            ++nbFill;
            if (mSizes[dim][i] == Layout::AUTO_SET ||
                sizes[i] <= Layout::AUTO_DEF)
            {
                sizes[i] = 0;
            }
        }
        upp -= sizes[i] + mSpacing;
    }
    upp = upp + mSpacing;

    if (nbFill == 0) return sizes;

    for (int i = 0; i < nb; ++i)
    {
        if (mSizes[dim][i] > Layout::AUTO_DEF) continue;
        int s = upp / nbFill;
        sizes[i] += s;
        upp -= s;
        --nbFill;
    }

    return sizes;
}

int LayoutArray::getSize(int dim) const
{
    std::vector< short > sizes = getSizes(dim, Layout::AUTO_DEF);
    int size = 0;
    int nb = sizes.size();
    for (int i = 0; i < nb; ++i)
    {
        if (sizes[i] > Layout::AUTO_DEF) size += sizes[i];
        size += mSpacing;
    }
    return size - mSpacing;
}

void LayoutArray::reflow(int nx, int ny, int nw, int nh)
{
    int gridW = mSizes[0].size(), gridH = mSizes[1].size();

    std::vector< short > widths  = getSizes(0, nw);
    std::vector< short > heights = getSizes(1, nh);

    int y = ny;
    for (int gridY = 0; gridY < gridH; ++gridY)
    {
        int x = nx;
        for (int gridX = 0; gridX < gridW; ++gridX)
        {
            LayoutCell *cell = mCells[gridY][gridX];
            if (cell && cell->mType != LayoutCell::NONE)
            {
                int dx = x, dy = y, dw, dh;
                align(dx, dw, 0, *cell, &widths[gridX]);
                align(dy, dh, 1, *cell, &heights[gridY]);
                cell->reflow(dx, dy, dw, dh);
            }
            x += widths[gridX] + mSpacing;
        }
        y += heights[gridY] + mSpacing;
    }
}

Layout::Layout(): mComputed(false)
{
    getArray();
    setPadding(6);
}

void Layout::reflow(int &nw, int &nh)
{
    if (!mComputed)
    {
        computeSizes();
        mComputed = true;
    }

    nw = nw == 0 ? mSize[0] + 2 * mPadding : nw;
    nh = nh == 0 ? mSize[1] + 2 * mPadding : nh;
    LayoutCell::reflow(0, 0, nw, nh);
}
