/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include <guichan/deathlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>

#include <list>
#include <map>
#include <memory>

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
                      public gcn::WidgetListener,
                      public gcn::DeathListener
{
    public:
        /**
         * Constructor. Initializes the graphic.
         *
         * @param inventory
         */
        ItemContainer(Inventory *inventory);
        ~ItemContainer() override;

        void hidePopup();

        /**
         * Necessary for checking how full the inventory is.
         */
        void logic() override;

        /**
         * Draws the items.
         */
        void draw(gcn::Graphics *graphics) override;

        // Overridden to disable drawing of the frame
        void drawFrame(gcn::Graphics *graphics) override {}

        // KeyListener
        void keyPressed(gcn::KeyEvent &event) override;
        void keyReleased(gcn::KeyEvent &event) override;

        // MouseListener
        void mousePressed(gcn::MouseEvent &event) override;
        void mouseDragged(gcn::MouseEvent &event) override;
        void mouseReleased(gcn::MouseEvent &event) override;
        void mouseMoved(gcn::MouseEvent &event) override;
        void mouseExited(gcn::MouseEvent &event) override;

        // WidgetListener
        void widgetResized(const gcn::Event &event) override;

        // DeathListener
        void death(const gcn::Event &event) override;

        /**
         * Returns the selected item.
         */
        Item *getSelectedItem() const;

        /**
         * Sets selected item to NULL.
         */
        void selectNone();


        /**
         * Sets item filter
         */
        void setFilter(const std::string &filter);

        void addSelectionListener(gcn::SelectionListener *listener);
        void removeSelectionListener(gcn::SelectionListener *listener);

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

        Item *getItemAt(int) const;

        Inventory *mInventory;
        int mGridColumns = 1;
        int mGridRows = 1;
        int mSelectedIndex = -1;
        int mHighlightedIndex = -1;
        int mLastUsedSlot = -1;
        SelectionState mSelectionStatus = SEL_NONE;
        bool mSwapItems = false;
        bool mDescItems = false;
        int mDragPosX = 0;
        int mDragPosY = 0;

        std::map<int, Item*> mFilteredMap;

        std::string mFilter;

        std::unique_ptr<ItemPopup> mItemPopup;

        std::list<gcn::SelectionListener *> mSelectionListeners;
};

/**
 * Adds a listener to the list that's notified each time a change to
 * the selection occurs.
 */
inline void ItemContainer::addSelectionListener(gcn::SelectionListener *listener)
{
    mSelectionListeners.push_back(listener);
}

/**
 * Removes a listener from the list that's notified each time a change
 * to the selection occurs.
 */
inline void ItemContainer::removeSelectionListener(gcn::SelectionListener *listener)
{
    mSelectionListeners.remove(listener);
}
