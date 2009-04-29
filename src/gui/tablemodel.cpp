/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
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

#include "tablemodel.h"

#include "utils/dtor.h"

#include <guichan/widget.hpp>

void TableModel::installListener(TableModelListener *listener)
{
    listeners.insert(listener);
}

void TableModel::removeListener(TableModelListener *listener)
{
    listeners.erase(listener);
}

void TableModel::signalBeforeUpdate()
{
    for (std::set<TableModelListener *>::const_iterator it = listeners.begin(); it != listeners.end(); it++)
        (*it)->modelUpdated(false);
}

void TableModel::signalAfterUpdate()
{
    for (std::set<TableModelListener *>::const_iterator it = listeners.begin(); it != listeners.end(); it++)
        (*it)->modelUpdated(true);
}


#define WIDGET_AT(row, column) (((row) * mColumns) + (column))
#define DYN_SIZE(h) ((h) >= 0)  // determines whether this size is tagged for auto-detection

StaticTableModel::StaticTableModel(int row, int column) :
    mRows(row),
    mColumns(column),
    mHeight(1)
{
    mTableModel.resize(row * column, NULL);
    mWidths.resize(column, 1);
}

StaticTableModel::~StaticTableModel()
{
    delete_all(mTableModel);
}

void StaticTableModel::resize()
{
    mRows = getRows();
    mColumns = getColumns();
    mTableModel.resize(mRows * mColumns, NULL);
}

void StaticTableModel::set(int row, int column, gcn::Widget *widget)
{
    if (row >= mRows || row < 0
        || column >= mColumns || column < 0)
        // raise exn?
        return;

    if (DYN_SIZE(mHeight)
        && widget->getHeight() > mHeight)
        mHeight = widget->getHeight();

    if (DYN_SIZE(mWidths[column])
        && widget->getWidth() > mWidths[column])
        mWidths[column] = widget->getWidth();

    signalBeforeUpdate();

    if (mTableModel[WIDGET_AT(row, column)])
        delete mTableModel[WIDGET_AT(row, column)];

    mTableModel[WIDGET_AT(row, column)] = widget;

    signalAfterUpdate();
}

gcn::Widget *StaticTableModel::getElementAt(int row, int column) const
{
    return mTableModel[WIDGET_AT(row, column)];
}

void StaticTableModel::fixColumnWidth(int column, int width)
{
    if (width < 0
        || column < 0 || column >= mColumns)
        return;

    mWidths[column] = -width; // Negate to tag as fixed
}

void StaticTableModel::fixRowHeight(int height)
{
    if (height < 0)
        return;

    mHeight = -height;
}

int StaticTableModel::getRowHeight() const
{
    return abs(mHeight);
}

int StaticTableModel::getColumnWidth(int column) const
{
   if (column < 0 || column >= mColumns)
        return 0;

    return abs(mWidths[column]);
}

int StaticTableModel::getRows() const
{
    return mRows;
}

int StaticTableModel::getColumns() const
{
    return mColumns;
}

int StaticTableModel::getWidth() const
{
    int width = 0;

    for (unsigned int i = 0; i < mWidths.size(); i++)
    {
        width += mWidths[i];
    }

    return width;
}

int StaticTableModel::getHeight() const
{
    return mColumns * mHeight;
}

