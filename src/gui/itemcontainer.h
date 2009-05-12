/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#ifndef ITEMCONTAINER_H
#define ITEMCONTAINER_H

#include <guichan/keylistener.hpp>
#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>

#include <list>

class Image;
class Inventory;
class Item;
class ItemPopup;

namespace gcn {
    class SelectionListener;
}

/**
 * An item container. Used to show items in inventory and trade dialog.
 *
 * \ingroup GUI
 */
class ItemContainer : public gcn::Widget,
                      public gcn::KeyListener,
                      public gcn::MouseListener,
                      public gcn::WidgetListener
{
    public:
        /**
         * Constructor. Initializes the graphic.
         *
         * @param inventory
         * @param gridColumns Amount of columns in grid.
         * @param gridRows    Amount of rows in grid.
         * @param offset      Index offset
         */
        ItemContainer(Inventory *inventory, bool forceQuantity = false);

        /**
         * Destructor.
         */
        virtual ~ItemContainer();

        /**
         * Necessary for checking how full the inventory is.
         */
        void logic();

        /**
         * Draws the items.
         */
        void draw(gcn::Graphics *graphics);

        // KeyListener
        void keyPressed(gcn::KeyEvent &event);
        void keyReleased(gcn::KeyEvent &event);

        // MouseListener
        void mousePressed(gcn::MouseEvent &event);
        void mouseDragged(gcn::MouseEvent &event);
        void mouseReleased(gcn::MouseEvent &event);
        void mouseMoved(gcn::MouseEvent &event);
        void mouseExited(gcn::MouseEvent &event);

        // WidgetListener
        void widgetResized(const gcn::Event &event);

        /**
         * Returns the selected item.
         */
        Item *getSelectedItem() const;

        /**
         * Sets selected item to NULL.
         */
        void selectNone();

        /**
         * Adds a listener to the list that's notified each time a change to
         * the selection occurs.
         */
        void addSelectionListener(gcn::SelectionListener *listener)
        {
            mSelectionListeners.push_back(listener);
        }

        /**
         * Removes a listener from the list that's notified each time a change
         * to the selection occurs.
         */
        void removeSelectionListener(gcn::SelectionListener *listener)
        {
            mSelectionListeners.remove(listener);
        }

    private:
        enum Direction
        {
            Left,
            Right,
            Up,
            Down
        };

        enum SelectionState
        {
            SEL_NONE = 0,
            SEL_SELECTED,
            SEL_SELECTING,
            SEL_DESELECTING,
            SEL_DRAGGING
        };

        /**
         * Execute all the functionality associated with the action key.
         */
        void keyAction();

        /**
         * Moves the highlight in the direction specified.
         *
         * @param direction The move direction of the highlighter.
         */
        void moveHighlight(Direction direction);

        /**
         * Sets the currently selected item.
         */
        void setSelectedIndex(int index);

        /**
         * Determine and set the height of the container.
         */
        void adjustHeight();

        /**
         * Sends out selection events to the list of selection listeners.
         */
        void distributeValueChangedEvent();

        /**
         * Gets the slot index based on the cursor position.
         *
         * @param x The X coordinate position.
         * @param y The Y coordinate position.
         * @return The slot index on success, -1 on failure.
         */
        int getSlotIndex(int x, int y) const;

        Inventory *mInventory;
        int mGridColumns, mGridRows;
        Image *mSelImg;
        int mSelectedIndex, mHighlightedIndex;
        int mLastUsedSlot;
        SelectionState mSelectionStatus;
        bool mForceQuantity;
        bool mSwapItems;
        bool mDescItems;
        int mDragPosX, mDragPosY;

        ItemPopup *mItemPopup;

        typedef std::list<gcn::SelectionListener*> SelectionListenerList;
        typedef SelectionListenerList::iterator SelectionListenerIterator;

        SelectionListenerList mSelectionListeners;
};

#endif
