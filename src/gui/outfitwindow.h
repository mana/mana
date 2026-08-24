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
#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

#include <vector>

constexpr int OUTFITS_COUNT = 15;

class Button;
class CheckBox;
class Label;
class OutfitContainer;

class OutfitWindow : public Window,
                     public DragTarget,
                     public gcn::ActionListener
{
    public:
        OutfitWindow();
        ~OutfitWindow() override;

        void action(const gcn::ActionEvent &event) override;

        void logic() override;

        void mousePressed(gcn::MouseEvent &event) override;

        bool handleDrop(const Drag &drag, int absX, int absY) override;

        void load();

        void setItemSelected(int itemId)
        { mItemSelected = itemId; }

        int getItemSelected() const
        { return mItemSelected; }

        bool isItemSelected() const
        { return mItemSelected > -1; }

        /**
         * The number of items in the current outfit.
         */
        int getItemCount() const
        { return static_cast<int>(mOutfits[mCurrentOutfit].items.size()); }

        int getItem(int index) const;

        /**
         * Inserts an item at the given index, or appends it when the index is
         * -1 or beyond the last item. Items already in the outfit are moved
         * rather than added again.
         */
        bool insertItem(int itemId, int index);

        void moveItem(int fromIndex, int toIndex);
        void removeItem(int index);

        void wearOutfit(int outfit);
        void copyOutfit(int outfit);

    private:
        void save();

        /**
         * Returns the items to equip for the given outfit. When it holds more
         * items of a kind than can be worn at the same time, the ones after
         * the currently equipped ones are chosen, so that wearing the outfit
         * again cycles through them.
         */
        std::vector<int> itemsToEquip(int outfit) const;

        void unequip(const std::vector<int> &exceptItems);

        struct Outfit
        {
            std::vector<int> items;
            bool unequip = true;
        };

        OutfitContainer *mOutfitContainer;
        Button *mPreviousButton;
        Button *mNextButton;
        Label *mCurrentLabel;
        CheckBox *mUnequipCheck;
        Button *mEquipButton;

        Outfit mOutfits[OUTFITS_COUNT];
        int mItemSelected = -1;
        int mCurrentOutfit = 0;
};

extern OutfitWindow *outfitWindow;
