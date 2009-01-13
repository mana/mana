/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
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
 */

#ifndef _TMW_ITEMCONTAINER_H__
#define _TMW_ITEMCONTAINER_H__

#include <guichan/keylistener.hpp>
#include <guichan/mouselistener.hpp>

#include <guichan/widget.hpp>

#include "itempopup.h"

#include <list>

class Image;
class Inventory;
class Item;

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
                      public gcn::MouseListener
{
    public:
        /**
         * Constructor. Initializes the graphic.
         * @param inventory
         * @param gridColumns Amount of columns in grid.
         * @param gridRows Amount of rows in grid.
         */
        ItemContainer(Inventory *inventory, int gridColumns, int gridRows);

        /**
         * Destructor.
         */
        virtual ~ItemContainer();

        /**
         * Draws the items.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Handles the key presses.
         */
        void keyPressed(gcn::KeyEvent &event);

        /**
         * Handles the key releases.
         */
        void keyReleased(gcn::KeyEvent &event);

        /**
         * Handles mouse click.
         */
        void mousePressed(gcn::MouseEvent &event);

        /**
         * Handles mouse dragged.
         */
        void mouseDragged(gcn::MouseEvent &event);

        /**
         * Handles mouse released.
         */
        void mouseReleased(gcn::MouseEvent &event);

        /**
         * Returns the selected item.
         */
        Item* getItem() const
        { return mSelectedItem; }

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

        enum {
            MOVE_SELECTED_LEFT,  // 0
            MOVE_SELECTED_RIGHT, // 1
            MOVE_SELECTED_UP,    // 2
            MOVE_SELECTED_DOWN   // 3
        };
    private:
        /**
         * Execute all the functionality associated with the action key.
         */
        void keyAction();

        void mouseEntered(gcn::MouseEvent &event);
        void mouseExited(gcn::MouseEvent &event);
        void mouseMoved(gcn::MouseEvent &event);

        /**
         * Moves the highlight in the direction specified.
         *
         * @param direction The move direction of the highlighter.
         */
        void moveHighlight(int direction);

        /**
         * Sets the currently selected item.
         */
        void setSelectedItem(Item *item);

        /**
         * Sends out selection events to the list of selection listeners.
         */
        void distributeValueChangedEvent();

        /**
         * Gets the slot index based on the cursor position.
         *
         * @param posX The X Coordinate position.
         * @param posY The Y Coordinate position.
         * @return The slot index on success, -1 on failure.
         */
        int getSlotIndex(const int posX, const int posY) const;

        Inventory *mInventory;
        int mGridColumns, mGridRows;
        Image *mSelImg;
        Item *mSelectedItem, *mHighlightedItem;
        int mSelectionStatus;
        bool mSwapItems;
        bool mDescItems;
        int mDragPosX, mDragPosY;

        ItemPopup *mItemPopup;

        typedef std::list<gcn::SelectionListener*> SelectionListenerList;
        typedef SelectionListenerList::iterator SelectionListenerIterator;

        SelectionListenerList mSelectionListeners;
};

#endif
