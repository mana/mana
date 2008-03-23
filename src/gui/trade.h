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

#ifndef _TMW_TRADE_H
#define _TMW_TRADE_H

#include <guichan/actionlistener.hpp>

#include "window.h"
#include "selectionlistener.h"

#include "../guichanfwd.h"

class Inventory;
class Item;
class ItemContainer;
class ScrollArea;

/**
 * Trade dialog.
 *
 * \ingroup Interface
 */
class TradeWindow : public Window, gcn::ActionListener, SelectionListener
{
    public:
        /**
         * Constructor.
         */
        TradeWindow();

        /**
         * Destructor.
         */
        ~TradeWindow();

        /**
         * Displays expected money in the trade window.
         */
        void setMoney(int quantity);

        /**
         * Add an item to the trade window.
         */
        void addItem(int id, bool own, int quantity);

        /**
         * Reset both item containers
         */
        void reset();

        /**
         * Player received ok message from server
         */
        void receivedOk();

        /**
         * Send trade packet.
         */
        void
        tradeItem(Item *item, int quantity);

        /**
         * Updates the labels and makes sure only one item is selected in
         * either my inventory or partner inventory.
         */
        void selectionChanged(const SelectionEvent &event);

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

    private:
        enum Status
        {
            PREPARING, /**< Players are adding items. */
            PROPOSING, /**< Local player is proposing a trade. */
            ACCEPTING, /**< Distant player is proposing a trade. */
        };

        /**
         * Sets the current status of the trade.
         */
        void setStatus(Status);

        Inventory *mMyInventory;
        Inventory *mPartnerInventory;

        ItemContainer *mMyItemContainer;
        ItemContainer *mPartnerItemContainer;

        gcn::Label *mItemNameLabel;
        gcn::Label *mItemDescriptionLabel;
        gcn::Label *mMoneyLabel;
        gcn::Button *mTradeButton;
        gcn::TextField *mMoneyField;

        Status mStatus;
};

extern TradeWindow *tradeWindow;

#endif
