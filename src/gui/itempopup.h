/*
 *  The Mana World
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008  The Mana World Development Team
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

#ifndef ITEMPOPUP_H
#define ITEMPOPUP_H

#include "gui/widgets/popup.h"

#include "resources/iteminfo.h"

#include <guichan/mouselistener.hpp>

class TextBox;

/**
 * A popup that displays information about an item.
 */
class ItemPopup : public Popup,
                  public gcn::MouseListener
{
    public:
        /**
         * Constructor. Initializes the item popup.
         */
        ItemPopup();

        /**
         * Destructor. Cleans up the item popup on deletion.
         */
        ~ItemPopup();

        /**
         * Sets the info to be displayed given a particular item.
         */
        void setItem(const ItemInfo &item);

        /**
         * Sets the location to display the item popup.
         */
        void view(int x, int y);

        void mouseMoved(gcn::MouseEvent &mouseEvent);

    private:
        gcn::Label *mItemName;
        TextBox *mItemDesc;
        TextBox *mItemEffect;
        TextBox *mItemWeight;
        ItemType mItemType;

        static gcn::Color getColor(ItemType type);
};

#endif // ITEMPOPUP_H
