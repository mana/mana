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

#include <sstream>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "chat.h"
#include "inventorywindow.h"
#include "item_amount.h"
#include "itemcontainer.h"
#include "scrollarea.h"
#include "textfield.h"
#include "trade.h"

#include "widgets/layout.h"

#include "../inventory.h"
#include "../item.h"
#include "../localplayer.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"
#include "../utils/stringutils.h"

TradeWindow::TradeWindow(Network *network):
    Window(_("Trade: You")),
    mNetwork(network),
    mMyInventory(new Inventory(INVENTORY_SIZE, 2)),
    mPartnerInventory(new Inventory(INVENTORY_SIZE, 2))
{
    setWindowName(_("Trade"));
    setDefaultSize(342, 209, ImageRect::CENTER);
    setResizable(true);

    setMinWidth(342);
    setMinHeight(209);

    mAddButton = new Button(_("Add"), "add", this);
    mOkButton = new Button(_("Ok"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);
    mTradeButton = new Button(_("Trade"), "trade", this);

    mTradeButton->setEnabled(false);

    mMyItemContainer = new ItemContainer(mMyInventory.get(), 2);
    mMyItemContainer->setWidth(160);
    mMyItemContainer->addSelectionListener(this);

    mMyScroll = new ScrollArea(mMyItemContainer);

    mPartnerItemContainer = new ItemContainer(mPartnerInventory.get(), 2);
    mPartnerItemContainer->setWidth(160);
    mPartnerItemContainer->addSelectionListener(this);

    mPartnerScroll = new ScrollArea(mPartnerItemContainer);

    mMoneyLabel = new gcn::Label(strprintf(_("You get %d GP."), 0));
    mMoneyLabel2 = new gcn::Label(_("You give:"));
    mMoneyField = new TextField;
    mMoneyField->setWidth(50);

    place(1, 0, mMoneyLabel);
    place(0, 1, mMyScroll).setPadding(3);
    place(1, 1, mPartnerScroll).setPadding(3);
    ContainerPlacer place;
    place = getPlacer(0, 0);
    place(0, 0, mMoneyLabel2);
    place(1, 0, mMoneyField);
    place = getPlacer(0, 2);
    place(0, 0, mAddButton);
    place(1, 0, mOkButton);
    place(2, 0, mTradeButton);
    place(3, 0, mCancelButton);
    Layout &layout = getLayout();
    layout.extend(0, 2, 2, 1);
    layout.setRowHeight(1, Layout::AUTO_SET);
    layout.setRowHeight(2, 0);
    layout.setColWidth(0, Layout::AUTO_SET);
    layout.setColWidth(1, Layout::AUTO_SET);

    loadWindowState();
}

TradeWindow::~TradeWindow()
{
}

void TradeWindow::widgetResized(const gcn::Event &event)
{
    mMyItemContainer->setWidth(mMyScroll->getWidth());
    mPartnerItemContainer->setWidth(mPartnerScroll->getWidth());

    Window::widgetResized(event);
}

void TradeWindow::addMoney(int amount)
{
    mMoneyLabel->setCaption(strprintf(_("You get %d GP."), amount));
    mMoneyLabel->adjustSize();
}

void TradeWindow::addItem(int id, bool own, int quantity, bool equipment)
{
    if (own)
    {
        mMyItemContainer->setWidth(mMyScroll->getWidth());
        mMyInventory->addItem(id, quantity, equipment);
    }
    else
    {
        mPartnerItemContainer->setWidth(mPartnerScroll->getWidth());
        mPartnerInventory->addItem(id, quantity, equipment);
    }
}

void TradeWindow::removeItem(int id, bool own)
{
    if (own)
        mMyInventory->removeItem(id);
    else
        mPartnerInventory->removeItem(id);
}

void TradeWindow::changeQuantity(int index, bool own, int quantity)
{
    if (own)
        mMyInventory->getItem(index)->setQuantity(quantity);
    else
        mPartnerInventory->getItem(index)->setQuantity(quantity);
}

void TradeWindow::increaseQuantity(int index, bool own, int quantity)
{
    if (own)
        mMyInventory->getItem(index)->increaseQuantity(quantity);
    else
        mPartnerInventory->getItem(index)->increaseQuantity(quantity);
}

void TradeWindow::reset()
{
    mMyInventory->clear();
    mPartnerInventory->clear();
    mTradeButton->setEnabled(false);
    mOkButton->setEnabled(true);
    mOkOther = false;
    mOkMe = false;
    mMoneyLabel->setCaption(strprintf(_("You get %d GP."), 0));
    mMoneyField->setEnabled(true);
    mMoneyField->setText("");
}

void TradeWindow::setTradeButton(bool enabled)
{
    mTradeButton->setEnabled(enabled);
}

void TradeWindow::receivedOk(bool own)
{
    if (own)
    {
        mOkMe = true;
        if (mOkOther)
        {
            mTradeButton->setEnabled(true);
            mOkButton->setEnabled(false);
        }
        else
        {
            mTradeButton->setEnabled(false);
            mOkButton->setEnabled(false);
        }
    }
    else
    {
        mOkOther = true;
        if (mOkMe)
        {
            mTradeButton->setEnabled(true);
            mOkButton->setEnabled(false);
        }
        else
        {
            mTradeButton->setEnabled(false);
            mOkButton->setEnabled(true);
        }
    }
}

void TradeWindow::tradeItem(Item *item, int quantity)
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_TRADE_ITEM_ADD_REQUEST);
    outMsg.writeInt16(item->getInvIndex());
    outMsg.writeInt32(quantity);
}

void TradeWindow::valueChanged(const gcn::SelectionEvent &event)
{
    const Item *item;

    /* If an item is selected in one container, make sure no item is selected
     * in the other container.
     */
    if (event.getSource() == mMyItemContainer &&
            (item = mMyItemContainer->getSelectedItem()))
        mPartnerItemContainer->selectNone();
    else if ((item = mPartnerItemContainer->getSelectedItem()))
        mMyItemContainer->selectNone();
}

void TradeWindow::action(const gcn::ActionEvent &event)
{
    Item *item = inventoryWindow->getSelectedItem();

    if (event.getId() == "add")
    {
        if (!item)
            return;

        if (mMyInventory->getFreeSlot() < 1)
            return;

        if (mMyInventory->contains(item))
        {
            chatWindow->chatLog(_("Failed adding item. You can not "
                        "overlap one kind of item on the window."), BY_SERVER);
            return;
        }

        if (item->getQuantity() == 1)
        {
            tradeItem(item, 1);
        }
        else
        {
            // Choose amount of items to trade
            new ItemAmountWindow(AMOUNT_TRADE_ADD, this, item);
        }
    }
    else if (event.getId() == "cancel")
    {
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_TRADE_CANCEL_REQUEST);
    }
    else if (event.getId() == "ok")
    {
        std::stringstream tempMoney(mMoneyField->getText());
        int tempInt;
        if (tempMoney >> tempInt)
        {
            mMoneyField->setText(toString(tempInt));

            MessageOut outMsg(mNetwork);
            outMsg.writeInt16(CMSG_TRADE_ITEM_ADD_REQUEST);
            outMsg.writeInt16(0);
            outMsg.writeInt32(tempInt);
        }
        else
        {
            mMoneyField->setText("");
        }
        mMoneyField->setEnabled(false);
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_TRADE_ADD_COMPLETE);
    }
    else if (event.getId() == "trade")
    {
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_TRADE_OK);
    }
}
