/*
 *  Aethyra
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Legend of Mazzeroth.
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

#ifndef ITEMPOPUP_H
#define ITEMPOPUP_H

#include "window.h"

class ItemInfo;
class ScrollArea;
class TextBox;

class ItemPopup : public Window
{
    public:
        ItemPopup();
        ~ItemPopup();

        void setItem(const ItemInfo &item);
        unsigned int getNumRows();
        void view(int x, int y);

    private:
        gcn::Label *mItemName;
        TextBox *mItemDesc;
        TextBox *mItemEffect;
        TextBox *mItemWeight;
        ScrollArea *mItemDescScroll;
        ScrollArea *mItemEffectScroll;
        ScrollArea *mItemWeightScroll;

        gcn::Color getColor(const std::string& type);
};

#endif // ITEMPOPUP_H
