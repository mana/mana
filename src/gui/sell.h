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
 *
 *  $Id$
 */

#ifndef _TMW_SELL_H
#define _TMW_SELL_H

#include <guichan/actionlistener.hpp>

#include "window.h"

#include "../guichanfwd.h"

class Item;
class Network;
class ShopItems;


/**
 * The sell dialog.
 *
 * \ingroup Interface
 */
class SellDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        SellDialog(Network *network);

        /**
         * Destructor
         */
        virtual ~SellDialog();

        /**
         * Resets the dialog, clearing inventory.
         */
        void reset();

        /**
         * Adds an item to the inventory.
         */
        void addItem(Item *item, int price);

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId);

        /**
         * Mouse callback
         */
        void mouseClick(int x, int y, int buton, int count);

    private:
        Network *mNetwork;
        gcn::Button *sellButton;
        gcn::Button *quitButton;
        gcn::Button *increaseButton;
        gcn::Button *decreaseButton;
        gcn::ListBox *itemList;
        gcn::ScrollArea *scrollArea;
        gcn::Label *moneyLabel;
        gcn::Label *itemDescLabel;
        gcn::Label *itemEffectLabel;
        gcn::Label *quantityLabel;
        gcn::Slider *slider;

        ShopItems *mShopItems;

        int m_maxItems;
        int m_amountItems;
};

#endif
