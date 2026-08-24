/*
 *  The Mana Client
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

#include <memory>

class Item;
class ItemPopup;
class OutfitWindow;

/**
 * Shows the items of the outfit currently selected in the outfit window.
 *
 * There is an item slot for each item in the outfit.
 *
 * \ingroup GUI
 */
class OutfitContainer : public ShortcutContainer,
                        public DragTarget
{
    public:
        OutfitContainer(OutfitWindow *outfitWindow);
        ~OutfitContainer() override;

        void draw(gcn::Graphics *graphics) override;

        void mouseDragged(gcn::MouseEvent &event) override;
        void mousePressed(gcn::MouseEvent &event) override;
        void mouseReleased(gcn::MouseEvent &event) override;
        void mouseMoved(gcn::MouseEvent &event) override;
        void mouseExited(gcn::MouseEvent &event) override;

        bool handleDrop(const Drag &drag, int absX, int absY) override;

    private:
        int getSlotItemId(int index) const override;
        void removeSlot(int index) override;

        OutfitWindow *mOutfitWindow;
        std::unique_ptr<ItemPopup> mItemPopup;
};
