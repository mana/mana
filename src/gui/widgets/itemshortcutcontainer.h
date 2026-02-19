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
#include "gui/widgets/shortcutcontainer.h"

#include <guichan/mouselistener.hpp>

#include <map>
#include <memory>

class Image;
class Item;
class ItemPopup;

/**
 * An item shortcut container. Used to quickly use items.
 *
 * \ingroup GUI
 */
class ItemShortcutContainer : public ShortcutContainer
                            , public DragTarget
                            , public DragSource
{
    public:
        ItemShortcutContainer();

        ~ItemShortcutContainer() override;

        /**
         * Draws the items.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Handles mouse when dragged.
         */
        void mouseDragged(gcn::MouseEvent &event) override;

        /**
         * Handles mouse when pressed.
         */
        void mousePressed(gcn::MouseEvent &event) override;

        /**
         * Handles mouse release.
         */
        void mouseReleased(gcn::MouseEvent &event) override;

        bool handleDrop(const Drag &drag, int absX, int absY) override;
        void dragFinished(const Drag &drag, DragResult result) override;

    private:
        void cleanupFallbackItems();
        void mouseExited(gcn::MouseEvent &event) override;
        void mouseMoved(gcn::MouseEvent &event) override;

        Item *getDisplayItem(int itemId);
        Item *getItemAt(int x, int y) const;

        int mClickedIndex = -1;

        std::map<int, std::unique_ptr<Item>> mFallbackItems;
        std::unique_ptr<ItemPopup> mItemPopup;
};
