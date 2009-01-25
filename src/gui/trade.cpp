/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#include "trade.h"

#include <sstream>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "chat.h"
#include "inventorywindow.h"
#include "item_amount.h"
#include "itemcontainer.h"
#include "scrollarea.h"
#include "textfield.h"

#include "../inventory.h"
#include "../item.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../resources/iteminfo.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"
#include "../utils/tostring.h"

TradeWindow::TradeWindow(Network *network):
    Window("Trade: You"),
    mNetwork(network),
    mMyInventory(new Inventory),
    mPartnerInventory(new Inventory)
{
    setWindowName("Trade");
    setDefaultSize(115, 227, 342, 209);
    setResizable(true);

    setMinWidth(342);
    setMinHeight(209);

    mAddButton = new Button(_("Add"), "add", this);
    mOkButton = new Button(_("Ok"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);
    mTradeButton = new Button(_("Trade"), "trade", this);

    mMyItemContainer = new ItemContainer(mMyInventory.get());
    mMyItemContainer->addSelectionListener(this);
    mMyItemContainer->setPosition(2, 2);

    mMyScroll = new ScrollArea(mMyItemContainer);
    mMyScroll->setPosition(8, 8);

    mPartnerItemContainer = new ItemContainer(mPartnerInventory.get());
    mPartnerItemContainer->addSelectionListener(this);
    mPartnerItemContainer->setPosition(2, 58);

    mPartnerScroll = new ScrollArea(mPartnerItemContainer);
    mPartnerScroll->setPosition(8, 64);

    mMoneyLabel = new gcn::Label(strprintf(_("You get %d GP."), 0));
    mMoneyLabel2 = new gcn::Label(_("You give:"));
    mMoneyField = new TextField;
    mMoneyField->setWidth(50);

    mAddButton->adjustSize();
    mOkButton->adjustSize();
    mCancelButton->adjustSize();
    mTradeButton->adjustSize();

    mTradeButton->setEnabled(false);

    mItemNameLabel = new gcn::Label(strprintf(_("Name: %s"), ""));
    mItemDescriptionLabel = new gcn::Label(
        strprintf(_("Description: %s"), ""));

    add(mMyScroll);
    add(mPartnerScroll);
    add(mAddButton);
    add(mOkButton);
    add(mCancelButton);
    add(mTradeButton);
    add(mItemNameLabel);
    add(mItemDescriptionLabel);
    add(mMoneyLabel2);
    add(mMoneyField);
    add(mMoneyLabel);

    loadWindowState();
}

TradeWindow::~TradeWindow()
{
}

void TradeWindow::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);

    const gcn::Rectangle &area = getChildrenArea();
    const int width = area.width;
    const int height = area.height;

    mMoneyLabel2->setPosition(8, height - 8 - mMoneyLabel2->getHeight());
    mMoneyField->setPosition(
        8 + mMoneyLabel2->getWidth(),
        height - 8 - mMoneyField->getHeight());
    mMoneyLabel->setPosition(
        mMoneyField->getX() + mMoneyField->getWidth() + 6,
        mMoneyLabel2->getY());

    mCancelButton->setPosition(width - 8 - mCancelButton->getWidth(),
                               height - 8 - mCancelButton->getHeight());
    mTradeButton->setPosition(
            mCancelButton->getX() - 4 - mTradeButton->getWidth(),
            mCancelButton->getY());
    mOkButton->setPosition(mTradeButton->getX() - 4 - mOkButton->getWidth(),
                           mCancelButton->getY());
    mAddButton->setPosition(mOkButton->getX() - 4 - mAddButton->getWidth(),
                            mCancelButton->getY());

    mItemDescriptionLabel->setPosition(8,
        mOkButton->getY() - mItemDescriptionLabel->getHeight() - 4);
    mItemNameLabel->setPosition(8,
        mItemDescriptionLabel->getY() - mItemNameLabel->getHeight() - 4);

    const int remaining = mItemNameLabel->getY() - 4 - 8 - 8;
    const int itemContainerHeight = remaining / 2;

    mMyItemContainer->setSize(width - 24 - 12 - 1,
        (INVENTORY_SIZE * 24) / (width / 24) - 1);
    mMyScroll->setSize(width - 16, itemContainerHeight);

    mPartnerItemContainer->setSize(width - 24 - 12 - 1,
        (INVENTORY_SIZE * 24) / (width / 24) - 1);
    mPartnerScroll->setSize(width - 16, itemContainerHeight);
    mPartnerScroll->setPosition(8, 8 + itemContainerHeight + 8);
}

void TradeWindow::addMoney(int amount)
{
    mMoneyLabel->setCaption(strprintf(_("You get %d GP."), amount));
    mMoneyLabel->adjustSize();
}

void TradeWindow::addItem(int id, bool own, int quantity, bool equipment)
{
    if (own) {
        mMyInventory->addItem(id, quantity, equipment);
    } else {
        mPartnerInventory->addItem(id, quantity, equipment);
    }
}

void TradeWindow::removeItem(int id, bool own)
{
    if (own) {
        mMyInventory->removeItem(id);
    } else {
        mPartnerInventory->removeItem(id);
    }
}

void TradeWindow::changeQuantity(int index, bool own, int quantity)
{
    if (own) {
        mMyInventory->getItem(index)->setQuantity(quantity);
    } else {
        mPartnerInventory->getItem(index)->setQuantity(quantity);
    }
}

void TradeWindow::increaseQuantity(int index, bool own, int quantity)
{
    if (own) {
        mMyInventory->getItem(index)->increaseQuantity(quantity);
    } else {
        mPartnerInventory->getItem(index)->increaseQuantity(quantity);
    }
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
    if (own) {
        mOkMe = true;
        if (mOkOther) {
            mTradeButton->setEnabled(true);
            mOkButton->setEnabled(false);
        } else {
            mTradeButton->setEnabled(false);
            mOkButton->setEnabled(false);
        }
    } else {
        mOkOther = true;
        if (mOkMe) {
            mTradeButton->setEnabled(true);
            mOkButton->setEnabled(false);
        } else {
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
    {
        mPartnerItemContainer->selectNone();
    }
    else if ((item = mPartnerItemContainer->getSelectedItem()))
    {
        mMyItemContainer->selectNone();
    }

    // Update name and description
    ItemInfo const *info = item ? &item->getInfo() : NULL;
    mItemNameLabel->setCaption(strprintf(_("Name: %s"),
        info ? info->getName().c_str() : ""));
    mItemNameLabel->adjustSize();
    mItemDescriptionLabel->setCaption(strprintf(_("Description: %s"),
        info ? info->getDescription().c_str() : ""));
    mItemDescriptionLabel->adjustSize();
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

        if (mMyInventory->contains(item)) {
            chatWindow->chatLog(_("Failed adding item. You can not "
                        "overlap one kind of item on the window."), BY_SERVER);
            return;
        }

        if (item->getQuantity() == 1) {
            tradeItem(item, 1);
        }
        else {
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
        } else {
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
