/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
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

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

constexpr int OUTFITS_COUNT = 15;
constexpr int OUTFIT_ITEM_COUNT = 9;

class Button;
class CheckBox;
class Item;
class Label;

class OutfitWindow : public Window, gcn::ActionListener
{
    public:
        OutfitWindow();
        ~OutfitWindow() override;

        void action(const gcn::ActionEvent &event) override;

        void draw(gcn::Graphics *graphics) override;
        void mouseDragged(gcn::MouseEvent &event) override;
        void mousePressed(gcn::MouseEvent &event) override;
        void mouseReleased(gcn::MouseEvent &event) override;

        void load();

        void setItemSelected(int itemId)
        { mItemSelected = itemId; }

        bool isItemSelected() const
        { return mItemSelected > -1; }

        void wearOutfit(int outfit);
        void copyOutfit(int outfit);

        void unequipNotInOutfit(int outfit);

    private:
        Button *mPreviousButton;
        Button *mNextButton;
        Label *mCurrentLabel;
        CheckBox *mUnequipCheck;

        int getIndexFromGrid(int pointX, int pointY) const;

        int mBoxWidth = 33;
        int mBoxHeight = 33;
        int mCursorPosX, mCursorPosY;
        int mGridWidth = 3;
        int mGridHeight = 3;
        bool mItemClicked = false;
        Item *mItemMoved = nullptr;

        void save();

        struct Outfit
        {
            int items[OUTFIT_ITEM_COUNT];
            bool unequip = true;
        };

        Outfit mOutfits[OUTFITS_COUNT];
        int mItemSelected = -1;

        int mCurrentOutfit = 0;
};

extern OutfitWindow *outfitWindow;
