/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _TMW_ITEM_AMOUNT_WINDOW_H
#define _TMW_ITEM_AMOUNT_WINDOW_H

#include <iosfwd>

#include <guichan/actionlistener.hpp>

#include "window.h"

#include "../guichanfwd.h"

class IntTextField;
class Item;

#define AMOUNT_TRADE_ADD 1
#define AMOUNT_ITEM_DROP 2

/**
 * Window used for selecting the amount of items to drop or trade.
 *
 * \ingroup Interface
 */
class ItemAmountWindow : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        ItemAmountWindow(int usage, Window *parent, Item *item);

        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Sets default amount value.
         */
        void resetAmount();

    private:
        IntTextField *mItemAmountTextField;   /**< Item amount caption. */
        Item *mItem;

        /**
         * Item Amount buttons.
         */
        gcn::Slider *mItemAmountSlide;
};

#endif /* _TMW_ITEM_AMOUNT_WINDOW_H */
