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
        { mAlign[0] = a; return *this; }

        /**
         * Sets the vertical alignment of the cell content.
         */
        Cell &setVAlign(Alignment a)
        { mAlign[1] = a; return *this; }

    private:

        gcn::Widget *mWidget;
        int mPadding;
        int mExtent[2];
        Alignment mAlign[2];
};

/**
 * This class is an helper for setting the position of widgets. They are
 * positioned along the cells of a rectangular table.
 *
 * The size of a given table column can either be set manually or be chosen
 * from the widest widget of the column. An empty column has a FILL width,
 * which means it will be extended so that the layout fits its minimum width.
 *
 * The process is similar for table rows. By default, there is a padding of 4
 * pixels between rows and between columns.
 */
class Layout
{
    public:

        Layout(): mPadding(4), mX(0), mY(0), mW(0), mH(0) {}

        /**
         * Sets the minimum width of a column.
         * @note Setting the width to FILL and then placing a widget in the
         * column will reset the width to zero.
         */
        void setColWidth(int n, int w);

        /**
         * Sets the minimum height of a row.
         * @note Setting the height to FILL and then placing a widget in the
         * row will reset the height to zero.
         */
        void setRowHeight(int n, int h);

        /**
         * Matchs widths of two columns.
         */
        void matchColWidth(int n1, int n2);

        /**
         * Sets the minimum width of the layout.
         */
        void setWidth(int w)
        { mW = w; }

        /**
         * Sets the minimum height of the layout.
         */
        void setHeight(int h)
        { mH = h; }

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

        /**
         * Reflows the current layout. Then starts a new layout just below with
         * the same width.
         */
        void flush();

        enum
        {
            FILL = -42, /**< Expand until the layout as the expected size. */
        };

    private:

        /**
         * Gets the position and size of a widget along a given axis
         */
        void align(int &pos, int &size, int dim, Cell &cell, int *sizes);

        /**
         * Ensures the private vectors are large enough.
         */
        void resizeGrid(int w, int h);

        /**
         * Gets the column/row sizes along a given axis.
         */
        std::vector< int > compute(int dim, int upp);

        std::vector< int > mSizes[2];
        std::vector< std::vector < Cell > > mCells;

        int mPadding;
        int mX, mY, mW, mH, mNW, mNH;
};

#endif
