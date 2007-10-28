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

#include <sstream>

#include <guichan/font.hpp>
#include <guichan/widgets/label.hpp>

#include "trade.h"

#include "button.h"
#include "chat.h"
#include "inventorywindow.h"
#include "item_amount.h"
#include "itemcontainer.h"
#include "scrollarea.h"
#include "textfield.h"

#include "widgets/layout.h"

#include "../inventory.h"
#include "../item.h"
#include "../localplayer.h"

#include "../net/gameserver/player.h"

#include "../resources/iteminfo.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"

TradeWindow::TradeWindow():
    Window("Trade: You"),
    mMyInventory(new Inventory),
    mPartnerInventory(new Inventory),
    mStatus(PREPARING)
{
    setResizable(true);
    setDefaultSize(115, 197, 332, 209);

    Button *mAddButton = new Button(_("Add"), "add", this);
    Button *mCancelButton = new Button(_("Cancel"), "cancel", this);
    mTradeButton = new Button(_("Propose trade"), "trade", this);
    mTradeButton->setWidth(8 + std::max(
        mTradeButton->getFont()->getWidth(_("Propose trade")),
        mTradeButton->getFont()->getWidth(_("Confirm trade"))));

    mMyItemContainer = new ItemContainer(mMyInventory, 4, 3);
    mMyItemContainer->addSelectionListener(this);
    ScrollArea *mMyScroll = new ScrollArea(mMyItemContainer);

    mPartnerItemContainer = new ItemContainer(mPartnerInventory, 4, 3);
    mPartnerItemContainer->addSelectionListener(this);
    ScrollArea *mPartnerScroll = new ScrollArea(mPartnerItemContainer);

    mMoneyLabel = new gcn::Label(strprintf(_("You get %d GP."), 0));
    gcn::Label *mMoneyLabel2 = new gcn::Label(_("You give:"));
    mMoneyField = new TextField;
    mMoneyField->setWidth(40);
    Button *mMoneyChange = new Button(_("Change"), "money", this);

    mItemNameLabel = new gcn::Label(strprintf(_("Name: %s"), ""));
    mItemDescriptionLabel = new gcn::Label(
        strprintf(_("Description: %s"), ""));

    place(1, 0, mMoneyLabel);
    place(0, 1, mMyScroll).setPadding(3);
    place(1, 1, mPartnerScroll).setPadding(3);
    ContainerPlacer place;
    place = getPlacer(0, 0);
    place(0, 0, mMoneyLabel2);
    place(1, 0, mMoneyField);
    place(2, 0, mMoneyChange).setHAlign(LayoutCell::LEFT);
    place = getPlacer(0, 2);
    place(0, 0, mItemNameLabel, 4);
    place(0, 1, mItemDescriptionLabel, 4);
    place(0, 2, mAddButton);
    place(2, 2, mTradeButton);
    place(3, 2, mCancelButton);
    Layout &layout = getLayout();
    layout.extend(0, 2, 2, 1);
    layout.setRowHeight(1, Layout::AUTO_SET);
    layout.setRowHeight(2, 0);
    layout.setColWidth(0, Layout::AUTO_SET);
    layout.setColWidth(1, Layout::AUTO_SET);

    loadWindowState("Trade");
}

TradeWindow::~TradeWindow()
{
    delete mMyInventory;
    delete mPartnerInventory;
}

void TradeWindow::setMoney(int amount)
{
    mMoneyLabel->setCaption(strprintf(_("You get %d GP."), amount));
    setStatus(PREPARING);
}

void TradeWindow::addItem(int id, bool own, int quantity)
{
    (own ? mMyInventory : mPartnerInventory)->addItem(id, quantity);
    setStatus(PREPARING);
}

void TradeWindow::reset()
{
    mMyInventory->clear();
    mPartnerInventory->clear();
    mMoneyLabel->setCaption(strprintf(_("You get %d GP."), 0));
    mMoneyField->setEnabled(true);
    mMoneyField->setText("");
    setStatus(PREPARING);
}

void TradeWindow::receivedOk()
{
    setStatus(ACCEPTING);
}

void TradeWindow::tradeItem(Item *item, int quantity)
{
    Net::GameServer::Player::tradeItem(item->getInvIndex(), quantity);
    addItem(item->getId(), true, quantity);
    item->increaseQuantity(-quantity);
}

void TradeWindow::selectionChanged(const SelectionEvent &event)
{
    Item *item;

    /* If an item is selected in one container, make sure no item is selected
     * in the other container.
     */
    if (event.getSource() == mMyItemContainer &&
            (item = mMyItemContainer->getItem()))
    {
        mPartnerItemContainer->selectNone();
    }
    else if ((item = mPartnerItemContainer->getItem()))
    {
        mMyItemContainer->selectNone();
    }

    // Update name and description
    ItemInfo const *info = item ? &item->getInfo() : NULL;
    mItemNameLabel->setCaption(strprintf(_("Name: %s"),
        info ? info->getName().c_str() : ""));
    mItemDescriptionLabel->setCaption(strprintf(_("Description: %s"),
        info ? info->getDescription().c_str() : ""));
}

void TradeWindow::setStatus(Status s)
{
    if (s == mStatus) return;
    mStatus = s;

    mTradeButton->setCaption
        (s == PREPARING ? _("Propose trade") : _("Confirm trade"));
    mTradeButton->setEnabled(s != PROPOSING);
}

void TradeWindow::action(const gcn::ActionEvent &event)
{
    Item *item = inventoryWindow->getItem();

    if (event.getId() == "add")
    {
        if (!item)
        {
            return;
        }

        if (mMyInventory->contains(item)) {
            chatWindow->chatLog("Failed adding item. You can not "
                    "overlap one kind of item on the window.", BY_SERVER);
            return;
        }

        if (item->getQuantity() == 1) {
            tradeItem(item, 1);
        }
        else {
            // Choose amount of items to trade
            new ItemAmountWindow(AMOUNT_TRADE_ADD, this, item);
        }

        setStatus(PREPARING);
    }
    else if (event.getId() == "cancel")
    {
        setVisible(false);
        reset();
        player_node->setTrading(false);
        Net::GameServer::Player::acceptTrade(false);
    }
    else if (event.getId() == "trade")
    {
        Net::GameServer::Player::acceptTrade(true);
        setStatus(PROPOSING);
    }
    else if (event.getId() == "money")
    {
        int v = atoi(mMoneyField->getText().c_str());
        Net::GameServer::Player::tradeMoney(v);
        mMoneyField->setText(strprintf("%d", v));
        setStatus(PREPARING);
    }
}
