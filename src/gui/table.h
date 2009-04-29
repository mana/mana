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

#ifndef TABLE_H
#define TABLE_H

#include "tablemodel.h"

#include <guichan/keylistener.hpp>
#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>

#include <vector>

class GuiTableActionListener;

/**
 * A table, with rows and columns made out of sub-widgets. Largely inspired by
 * (and can be thought of as a generalisation of) the guichan listbox
 * implementation.
 *
 * Normally you want this within a ScrollArea.
 *
 * \ingroup GUI
 */
class GuiTable : public gcn::Widget,
                 public gcn::MouseListener,
                 public gcn::KeyListener,
                 public TableModelListener
{
    // so that the action listener can call distributeActionEvent
    friend class GuiTableActionListener;

public:
    GuiTable(TableModel * initial_model = NULL, gcn::Color background = 0xffffff,
             bool opacity = true);

    virtual ~GuiTable();

    /**
     * Retrieves the active table model
     */
    TableModel *getModel() const;

    /**
     * Sets the table model
     *
     * Note that actions issued by widgets returned from the model will update
     * the table selection, but only AFTER any event handlers installed within
     * the widget have been triggered. To be notified after such an update, add
     * an action listener to the table instead.
     */
    void setModel(TableModel *m);

    void setSelected(int row, int column);

    int getSelectedRow() const;

    int getSelectedColumn() const;

    void setSelectedRow(int selected);

    void setSelectedColumn(int selected);

    bool isWrappingEnabled() const {return mWrappingEnabled;}

    void setWrappingEnabled(bool wrappingEnabled)
    {mWrappingEnabled = wrappingEnabled;}

    gcn::Rectangle getChildrenArea() const;

    /**
     * Toggle whether to use linewise selection mode, in which the table selects
     * an entire line at a time, rather than a single cell.
     *
     * Note that column information is tracked even in linewise selection mode;
     * this mode therefore only affects visualisation.
     *
     * Disabled by default.
     *
     * \param linewise: Whether to enable linewise selection mode
     */
    void setLinewiseSelection(bool linewise);

    // Inherited from Widget
    virtual void draw(gcn::Graphics* graphics);

    virtual gcn::Widget *getWidgetAt(int x, int y) const;

    virtual void moveToTop(gcn::Widget *child);

    virtual void moveToBottom(gcn::Widget *child);

    virtual void _setFocusHandler(gcn::FocusHandler* focusHandler);

    // Inherited from KeyListener
    virtual void keyPressed(gcn::KeyEvent& keyEvent);

    /**
     * Sets the table to be opaque, that is sets the table
     * to display its background.
     *
     * @param opaque True if the table should be opaque, false otherwise.
     */
    virtual void setOpaque(bool opaque) {mOpaque = opaque;}

    /**
     * Checks if the table is opaque, that is if the table area displays its
     * background.
     *
     * @return True if the table is opaque, false otherwise.
     */
    virtual bool isOpaque() const {return mOpaque;}

    // Inherited from MouseListener
    virtual void mousePressed(gcn::MouseEvent& mouseEvent);

    virtual void mouseWheelMovedUp(gcn::MouseEvent& mouseEvent);

    virtual void mouseWheelMovedDown(gcn::MouseEvent& mouseEvent);

    virtual void mouseDragged(gcn::MouseEvent& mouseEvent);

    // Constraints inherited from TableModelListener
    virtual void modelUpdated(bool);

protected:
    /** Frees all action listeners on inner widgets. */
    virtual void uninstallActionListeners();
    /** Installs all action listeners on inner widgets. */
    virtual void installActionListeners();

    virtual int getRowHeight() const;
    virtual int getColumnWidth(int i) const;

private:
    int getRowForY(int y) const; // -1 on error
    int getColumnForX(int x) const; // -1 on error
    void recomputeDimensions();
    bool mLinewiseMode;
    bool mWrappingEnabled;
    bool mOpaque;

    static float mAlpha;

    /**
     * Holds the background color of the table.
     */
    gcn::Color mBackgroundColor;

    TableModel *mModel;

    int mSelectedRow;
    int mSelectedColumn;

    /** Number of frames to skip upwards when drawing the selected widget. */
    int mPopFramesNr;

    /** If someone moves a fresh widget to the top, we must display it. */
    gcn::Widget *mTopWidget;

    /** Vector for compactness; used as a list in practice. */
    std::vector<GuiTableActionListener *> mActionListeners;
};


#endif /* !defined(TABLE_H) */
