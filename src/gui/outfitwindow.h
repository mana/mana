/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef OUTFITWINDOW_H
#define OUTFITWINDOW_H

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

#define OUTFITS_COUNT 15
#define OUTFIT_ITEM_COUNT 9

class Button;
class CheckBox;
class Item;
class Label;

class OutfitWindow : public Window, gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        OutfitWindow();

        /**
         * Destructor.
         */
        ~OutfitWindow();

        void action(const gcn::ActionEvent &event);

        void draw(gcn::Graphics *graphics);

        void mousePressed(gcn::MouseEvent &event);

        void mouseDragged(gcn::MouseEvent &event);

        void mouseReleased(gcn::MouseEvent &event);

        void load();

        void setItemSelected(int itemId)
        { mItemSelected = itemId; }

        bool isItemSelected()
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

        int mBoxWidth;
        int mBoxHeight;
        int mCursorPosX, mCursorPosY;
        int mGridWidth, mGridHeight;
        bool mItemClicked;
        Item *mItemMoved;

        void save();

        int mItems[OUTFITS_COUNT][OUTFIT_ITEM_COUNT];
        bool mItemsUnequip[OUTFITS_COUNT];
        int mItemSelected;

        int mCurrentOutfit;
};

extern OutfitWindow *outfitWindow;

#endif
