/*
 *  The Mana World
 *  Copyright (C) 2007  The Mana World Development Team
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

#ifndef OUTFITWINDOW_H
#define OUTFITWINDOW_H

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

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

        int mItems[10][9];
        int mItemSelected;

        int mCurrentOutfit;
};

extern OutfitWindow *outfitWindow;

#endif
