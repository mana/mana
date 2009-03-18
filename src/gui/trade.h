/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#ifndef TRADE_H
#define TRADE_H

#include <memory>

#include <guichan/actionlistener.hpp>
#include <guichan/selectionlistener.hpp>

#include "window.h"

#include "../guichanfwd.h"

class Inventory;
class Item;
class ItemContainer;
class Network;
class ScrollArea;

/**
 * Trade dialog.
 *
 * \ingroup Interface
 */
class TradeWindow : public Window, gcn::ActionListener, gcn::SelectionListener
{
    public:
        /**
         * Constructor.
         */
        TradeWindow(Network *network);

        /**
         * Destructor.
         */
        ~TradeWindow();

        /**
         * Add money to the trade window.
         */
        void addMoney(int quantity);

        /**
         * Add an item to the trade window.
         */
        void addItem(int id, bool own, int quantity, bool equipment);

        /**
         * Remove a item from the trade window.
         */
        void removeItem(int id, bool own);

        /**
         * Reset both item containers
         */
        void reset();

        /**
         * Change quantity of an item.
         */
        void changeQuantity(int index, bool own, int quantity);

        /**
         * Increase quantity of an item.
         */
        void increaseQuantity(int index, bool own, int quantity);

        /**
         * Set trade Button disabled
         */
        void setTradeButton(bool enabled);

        /**
         * Player received ok message from server
         */
        void receivedOk(bool own);

        /**
         * Send trade packet.
         */
        void tradeItem(Item *item, int quantity);

        /**
         * Updates the labels and makes sure only one item is selected in
         * either my inventory or partner inventory.
         */
        void valueChanged(const gcn::SelectionEvent &event);

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Closes the Trade Window, as well as telling the server that the
         * window has been closed.
         */
        void close();

    private:
        Network *mNetwork;

        typedef const std::auto_ptr<Inventory> InventoryPtr;
        InventoryPtr mMyInventory;
        InventoryPtr mPartnerInventory;

        ItemContainer *mMyItemContainer;
        ItemContainer *mPartnerItemContainer;

        gcn::Label *mMoneyLabel;
        gcn::Label *mMoneyLabel2;
        gcn::Button *mAddButton, *mOkButton, *mCancelButton, *mTradeButton;
        ScrollArea *mMyScroll, *mPartnerScroll;
        gcn::TextField *mMoneyField;
        bool mOkOther, mOkMe;
};

extern TradeWindow *tradeWindow;

#endif
