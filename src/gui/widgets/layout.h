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
         * Sets padding.
         */
        Cell &setPadding(int p)
        { mPadding = p; return *this; }

        /**
         * Sets horizontal alignment of cell content.
         */
        Cell &setHAlign(Alignment a)
        { mHAlign = a; return *this; }

        /**
         * Sets vertical alignment of cell content.
         */
        Cell &setVAlign(Alignment a)
        { mVAlign = a; return *this; }

    private:

        gcn::Widget *mWidget;
        int mColExtent, mRowExtent;
        int mPadding;
        Alignment mHAlign, mVAlign;
};

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
         * Sets padding between cells.
         */
        void setPadding(int p)
        { mPadding = p; }

        /**
         * Places a widget in given cell.
         */
        Cell &place(gcn::Widget *, int x, int y, int w, int h);

        /**
         * Computes position of all the widgets.
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
