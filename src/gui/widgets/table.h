/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

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
class GuiTable final : public gcn::Widget,
                       public gcn::MouseListener,
                       public gcn::KeyListener,
                       public TableModelListener
{
    // so that the action listener can call distributeActionEvent
    friend class GuiTableActionListener;

public:
    GuiTable(TableModel * initial_model = nullptr, gcn::Color background = 0xffffff,
             bool opacity = true);

    ~GuiTable() override;

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
    void draw(gcn::Graphics* graphics) override;

    // Overridden to disable drawing of the frame
    void drawFrame(gcn::Graphics* graphics) override {}

    virtual gcn::Widget *getWidgetAt(int x, int y) const;

    void moveToTop(gcn::Widget *child) override;

    void moveToBottom(gcn::Widget *child) override;

    void _setFocusHandler(gcn::FocusHandler* focusHandler) override;

    // Inherited from KeyListener
    void keyPressed(gcn::KeyEvent& keyEvent) override;

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
    void mousePressed(gcn::MouseEvent& mouseEvent) override;

    void mouseWheelMovedUp(gcn::MouseEvent& mouseEvent) override;

    void mouseWheelMovedDown(gcn::MouseEvent& mouseEvent) override;

    void mouseDragged(gcn::MouseEvent& mouseEvent) override;

    // Constraints inherited from TableModelListener
    void modelUpdated(bool) override;

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
