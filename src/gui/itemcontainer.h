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

#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>

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
class ItemContainer : public gcn::Widget, public gcn::MouseListener,
    public gcn::WidgetListener
{
    public:
        /**
         * Constructor. Initializes the graphic.
         */
        ItemContainer(Inventory *inventory);

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
        /**
         * Sets the currently selected item.  Invalid (e.g., negative) indices set `no item'.
         */
        void setSelectedItemIndex(int index);

        /**
         * Find the current item index by the most recently used item ID
         */
        void refindSelectedItem(void);

        /**
         * Determine and set the height of the container.
         */
        void recalculateHeight(void);

        /**
         * Sends out selection events to the list of selection listeners.
         */
        void distributeValueChangedEvent(void);

        Inventory *mInventory;
        Image *mSelImg;
        int mSelectedItemIndex;
        int mLastSelectedItemId;  // last selected item ID.  If we lose the item, find again by ID.

        int mMaxItems;

        std::list<gcn::SelectionListener*> mListeners;

        static const int gridWidth;
        static const int gridHeight;
};

#endif
