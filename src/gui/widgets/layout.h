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

#ifndef _TMW_WIDGET_LAYOUT_H__
#define _TMW_WIDGET_LAYOUT_H__

#include <vector>

#include <guichan/widget.hpp>

/**
 * This class describes the formatting of a widget in the cell of a layout
 * table. Horizontally, a widget can either fill the width of the cell (minus
 * the cell padding), or it can retain its size and be flushed left, or flush
 * right, or centered in the cell. The process is similar for the vertical
 * alignment, except that top is represented by LEFT and bottom by RIGHT.
 */
class Cell
{
    friend class Layout;

    public:

        enum Alignment
        {
            LEFT, RIGHT, CENTER, FILL
        };

        Cell(): mWidget(0) {}

        /**
         * Sets the padding around the cell content.
         */
        Cell &setPadding(int p)
        { mPadding = p; return *this; }

        /**
         * Sets the horizontal alignment of the cell content.
         */
        Cell &setHAlign(Alignment a)
        { mHAlign = a; return *this; }

        /**
         * Sets the vertical alignment of the cell content.
         */
        Cell &setVAlign(Alignment a)
        { mVAlign = a; return *this; }

    private:

        gcn::Widget *mWidget;
        int mColExtent, mRowExtent;
        int mPadding;
        Alignment mHAlign, mVAlign;
};

/**
 * This class is an helper for setting the position of widgets. They are
 * positioned along the cells of a rectangular table. The size of a given
 * table column can either be set manually or be chosen from the widest widget
 * of the column. The process is similar for table rows. By default, there is a
 * padding of 4 pixels between rows and between columns.
 */
class Layout
{
    public:

        Layout(): mPadding(4) {}

        /**
         * Sets the width of a column.
         */
        void setColWidth(int n, int w);

        /**
         * Sets the height of a row.
         */
        void setRowHeight(int n, int h);

        /**
         * Sets the padding between cells.
         */
        void setPadding(int p)
        { mPadding = p; }

        /**
         * Places a widget in a given cell.
         */
        Cell &place(gcn::Widget *, int x, int y, int w, int h);

        /**
         * Computes the positions of all the widgets.
         */
        void reflow();

    private:

        /**
         * Ensures the private vectors are large enough.
         */
        void resizeGrid(int w, int h);

        std::vector< int > mColWidths;
        std::vector< int > mRowHeights;
        std::vector< std::vector < Cell > > mCells;

        int mPadding;
};

#endif
