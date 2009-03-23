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

#include <sstream>

#include <guichan/font.hpp>
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
#include "../units.h"

#ifdef TMWSERV_SUPPORT
#include "../net/tmwserv/gameserver/player.h"
#else
#include "../net/messageout.h"
#include "../net/ea/protocol.h"
#endif

#include "../utils/gettext.h"
#include "../utils/stringutils.h"
#include "../utils/strprintf.h"

#ifdef TMWSERV_SUPPORT
TradeWindow::TradeWindow():
#else
TradeWindow::TradeWindow(Network *network):
#endif
    Window(_("Trade: You")),
#ifdef EATHENA_SUPPORT
    mNetwork(network),
#endif
    mMyInventory(new Inventory(INVENTORY_SIZE)),
    mPartnerInventory(new Inventory(INVENTORY_SIZE))
#ifdef TMWSERV_SUPPORT
    , mStatus(PREPARING)
#endif
{
    setWindowName("Trade");
    setResizable(true);
    setDefaultSize(115, 197, 332, 209);

    Button *mAddButton = new Button(_("Add"), "add", this);
#ifdef EATHENA_SUPPORT
    mOkButton = new Button(_("Ok"), "ok", this);
#endif
    Button *mCancelButton = new Button(_("Cancel"), "cancel", this);
    mTradeButton = new Button(_("Propose trade"), "trade", this);
    mTradeButton->setWidth(8 + std::max(
        mTradeButton->getFont()->getWidth(_("Propose trade")),
        mTradeButton->getFont()->getWidth(_("Confirm trade"))));

#ifdef TMWSERV_SUPPORT
    mMyItemContainer = new ItemContainer(mMyInventory.get(), 4, 3, 0);
#else
    mMyItemContainer = new ItemContainer(mMyInventory.get(), 4, 3, 2);
#endif
    mMyItemContainer->addSelectionListener(this);
    ScrollArea *mMyScroll = new ScrollArea(mMyItemContainer);

#ifdef TMWSERV_SUPPORT
    mPartnerItemContainer = new ItemContainer(mPartnerInventory.get(), 4, 3, 0);
#else
    mPartnerItemContainer = new ItemContainer(mPartnerInventory.get(), 4, 3, 2);
#endif
    mPartnerItemContainer->addSelectionListener(this);
    ScrollArea *mPartnerScroll = new ScrollArea(mPartnerItemContainer);

    mMoneyLabel = new gcn::Label(strprintf(_("You get %d GP."), 0));
    gcn::Label *mMoneyLabel2 = new gcn::Label(_("You give:"));
    mMoneyField = new TextField;
    mMoneyField->setWidth(40);
    Button *mMoneyChange = new Button(_("Change"), "money", this);

    place(1, 0, mMoneyLabel);
    place(0, 1, mMyScroll).setPadding(3);
    place(1, 1, mPartnerScroll).setPadding(3);
    ContainerPlacer place;
    place = getPlacer(0, 0);
    place(0, 0, mMoneyLabel2);
    place(1, 0, mMoneyField);
    place(2, 0, mMoneyChange).setHAlign(LayoutCell::LEFT);
    place = getPlacer(0, 2);
    place(0, 0, mAddButton);
#ifdef EATHENA_SUPPORT
    place(1, 0, mOkButton);
#endif
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

void TradeWindow::setMoney(int amount)
{
    mMoneyLabel->setCaption(strprintf(_("You get %s."),
                                       Units::formatCurrency(amount).c_str()));
    mMoneyLabel->adjustSize();
#ifdef TMWSERV_SUPPORT
    setStatus(PREPARING);
#endif
}

#ifdef TMWSERV_SUPPORT
void TradeWindow::addItem(int id, bool own, int quantity)
{
    (own ? mMyInventory : mPartnerInventory)->addItem(id, quantity);
    setStatus(PREPARING);
}
#endif

#ifdef EATHENA_SUPPORT
void TradeWindow::addItem(int id, bool own, int quantity, bool equipment)
{
    if (own)
    {
        mMyInventory->addItem(id, quantity, equipment);
    }
    else
    {
        mPartnerInventory->addItem(id, quantity, equipment);
    }
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
#endif

void TradeWindow::reset()
{
    mMyInventory->clear();
    mPartnerInventory->clear();
#ifdef EATHENA_SUPPORT
    mTradeButton->setEnabled(false);
    mOkButton->setEnabled(true);
    mOkOther = false;
    mOkMe = false;
#endif
    mMoneyLabel->setCaption(strprintf(_("You get %s."), ""));
    mMoneyField->setEnabled(true);
    mMoneyField->setText("");
#ifdef TMWSERV_SUPPORT
    setStatus(PREPARING);
#endif
}

#ifdef TMWSERV_SUPPORT

void TradeWindow::receivedOk()
{
    setStatus(ACCEPTING);
}

#else

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

#endif

void TradeWindow::tradeItem(Item *item, int quantity)
{
#ifdef TMWSERV_SUPPORT
    Net::GameServer::Player::tradeItem(item->getInvIndex(), quantity);
    addItem(item->getId(), true, quantity);
    item->increaseQuantity(-quantity);
#else
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_TRADE_ITEM_ADD_REQUEST);
    outMsg.writeInt16(item->getInvIndex());
    outMsg.writeInt32(quantity);
#endif
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

#ifdef TMWSERV_SUPPORT
void TradeWindow::setStatus(Status s)
{
    if (s == mStatus) return;
    mStatus = s;

    mTradeButton->setCaption
        (s == PREPARING ? _("Propose trade") : _("Confirm trade"));
    mTradeButton->setEnabled(s != PROPOSING);
}
#endif

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
            chatWindow->chatLog("Failed adding item. You can not "
                    "overlap one kind of item on the window.", BY_SERVER);
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

#ifdef TMWSERV_SUPPORT
        setStatus(PREPARING);
#endif
    }
    else if (event.getId() == "cancel")
    {
        setVisible(false);
        reset();
        player_node->setTrading(false);
#ifdef TMWSERV_SUPPORT
        Net::GameServer::Player::acceptTrade(false);
#else
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_TRADE_CANCEL_REQUEST);
#endif
    }
#ifdef EATHENA_SUPPORT
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
#endif
    else if (event.getId() == "trade")
    {
#ifdef TMWSERV_SUPPORT
        Net::GameServer::Player::acceptTrade(true);
        setStatus(PROPOSING);
#else
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_TRADE_OK);
#endif
    }
#ifdef TMWSERV_SUPPORT
    else if (event.getId() == "money")
    {
        int v = atoi(mMoneyField->getText().c_str());
        Net::GameServer::Player::tradeMoney(v);
        mMoneyField->setText(strprintf("%d", v));
        setStatus(PREPARING);
    }
#endif
}
