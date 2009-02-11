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

#include <list>

#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>

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
                      public gcn::MouseListener,
                      public gcn::WidgetListener
{
    public:
        /**
         * Constructor. Initializes the graphic.
         */
        ItemContainer(Inventory *inventory, int offset);

        /**
         * Destructor.
         */
        virtual ~ItemContainer();

        /**
         * Handles the logic of the ItemContainer
         */
        void logic();

        /**
         * Draws the items.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Called whenever the widget changes size.
         */
        void widgetResized(const gcn::Event &event);

        /**
         * Handles mouse click.
         */
        void mousePressed(gcn::MouseEvent &event);

        /**
         * Returns the selected item.
         */
        Item* getSelectedItem();

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
            mListeners.push_back(listener);
        }

        /**
         * Removes a listener from the list that's notified each time a change
         * to the selection occurs.
         */
        void removeSelectionListener(gcn::SelectionListener *listener)
        {
            mListeners.remove(listener);
        }

    private:
        void mouseExited(gcn::MouseEvent &event);
        void mouseMoved(gcn::MouseEvent &event);

        /**

         * Sets the currently selected item.  Invalid (e.g., negative) indices set `no item'.
         */
        void setSelectedItemIndex(int index);

        /**
         * Find the current item index by the most recently used item ID
         */
        void refindSelectedItem();

        /**
         * Determine and set the height of the container.
         */
        void recalculateHeight();

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
        Image *mSelImg;

        int mSelectedItemIndex;
        int mLastSelectedItemId;  // last selected item ID. If we lose the item, find again by ID.
        int mMaxItems;
        int mOffset;

        ItemPopup *mItemPopup;

        std::list<gcn::SelectionListener*> mListeners;

        static const int gridWidth;
        static const int gridHeight;
};

#endif
