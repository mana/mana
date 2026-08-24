/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "gui/dragndrop.h"

#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>

#include <map>
#include <memory>

class Item;

/**
 * A generic shortcut container.
 *
 * \ingroup GUI
 */
class ShortcutContainer : public gcn::Widget,
                          public gcn::WidgetListener,
                          public gcn::MouseListener,
                          public DragSource
{
    public:
        ShortcutContainer();
        ~ShortcutContainer() override;

        /**
         * Draws the shortcuts
         */
        void draw(gcn::Graphics *graphics) override = 0;

        // Overridden to disable drawing of the frame
        void drawFrame(gcn::Graphics *graphics) override {}

        /**
         * Invoked when a widget changes its size. This is used to determine
         * the new height of the container.
         */
        void widgetResized(const gcn::Event &event) override;

        int getMaxItems() const { return mMaxItems; }
        void setMaxItems(int maxItems);
        int getBoxWidth() const { return mBoxWidth; }
        int getBoxHeight() const { return mBoxHeight; }

        void dragFinished(const Drag &drag, DragResult result) override;

    protected:
        /**
         * Removes the shortcut at the given slot index.
         */
        virtual void removeSlot(int index) = 0;

        /**
         * Returns the ID of the item in the given slot, or -1 when the slot
         * holds no item.
         */
        virtual int getSlotItemId(int index) const { return -1; }

        /**
         * Returns the item to display for the given item ID. Items which are
         * not in the inventory get a temporary instance, so that they can be
         * displayed as well.
         */
        Item *getDisplayItem(int itemId);

        /**
         * Forgets the temporary items which are no longer referenced by any
         * slot, or which are back in the inventory.
         */
        void cleanupFallbackItems();

        /**
         * Gets the index from the grid provided the point is in an item box.
         *
         * @param pointX X coordinate of the point.
         * @param pointY Y coordinate of the point.
         * @return index on success, -1 on failure.
         */
        int getIndexFromGrid(int pointX, int pointY) const;

        /**
         * Gets the index from the grid for a point in absolute coordinates.
         *
         * @return index on success, -1 on failure.
         */
        int getIndexFromAbsolute(int absX, int absY) const;

        /**
         * Returns whether the given slot is the source of the active drag.
         */
        bool isSlotDragged(int index) const;

        int mClickedIndex = -1;
        int mMaxItems = 0;
        int mBoxWidth = 0;
        int mBoxHeight = 0;
        int mGridWidth = 1;
        int mGridHeight = 1;

    private:
        /**
         * Determines the grid size and the height of the container.
         */
        void updateGrid();

        std::map<int, std::unique_ptr<Item>> mFallbackItems;
};
