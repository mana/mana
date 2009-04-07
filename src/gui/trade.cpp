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

#include "gui/trade.h"

#include "inventory.h"
#include "item.h"
#include "localplayer.h"
#include "units.h"

#include "gui/inventorywindow.h"
#include "gui/item_amount.h"
#include "gui/itemcontainer.h"

#include "gui/widgets/button.h"
#include "gui/widgets/chattab.h"
#include "gui/widgets/label.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/textfield.h"
#include "gui/widgets/layout.h"

#include "net/net.h"
#include "net/tradehandler.h"
#ifdef TMWSERV_SUPPORT
#include "net/tmwserv/gameserver/player.h"
#else
#include "net/ea/tradehandler.h"
#endif

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/strprintf.h"

#include <sstream>

#include <guichan/font.hpp>

TradeWindow::TradeWindow():
    Window(_("Trade: You")),
    mMyInventory(new Inventory(INVENTORY_SIZE)),
    mPartnerInventory(new Inventory(INVENTORY_SIZE))
#ifdef TMWSERV_SUPPORT
    , mStatus(PREPARING)
#endif
{
    setWindowName("Trade");
    setResizable(false);
    setCloseButton(true);
    setDefaultSize(386, 180, ImageRect::CENTER);

    std::string longestName = getFont()->getWidth(_("OK")) >
                                   getFont()->getWidth(_("Trade")) ?
                                   _("OK") : _("Trade");

    Button *addButton = new Button(_("Add"), "add", this);
#ifdef EATHENA_SUPPORT
    mOkButton = new Button(longestName, "ok", this);
#else
    Button *cancelButton = new Button(_("Cancel"), "cancel", this);
#endif
    mTradeButton = new Button(_("Propose trade"), "trade", this);
    mTradeButton->setWidth(8 + std::max(
        mTradeButton->getFont()->getWidth(_("Propose trade")),
        mTradeButton->getFont()->getWidth(_("Confirm trade"))));

    mMyItemContainer = new ItemContainer(mMyInventory.get(), 5, 2);
    mMyItemContainer->addSelectionListener(this);

    ScrollArea *myScroll = new ScrollArea(mMyItemContainer);

    mPartnerItemContainer = new ItemContainer(mPartnerInventory.get(), 5, 2);
    mPartnerItemContainer->addSelectionListener(this);

    ScrollArea *partnerScroll = new ScrollArea(mPartnerItemContainer);

    mMoneyLabel = new Label(strprintf(_("You get %s."), ""));
    gcn::Label *mMoneyLabel2 = new Label(_("You give:"));

    mMoneyField = new TextField;
    mMoneyField->setWidth(40);
    Button *moneyChange = new Button(_("Change"), "money", this);

    place(1, 0, mMoneyLabel);
    place(0, 1, myScroll).setPadding(3);
    place(1, 1, partnerScroll).setPadding(3);
    ContainerPlacer place;
    place = getPlacer(0, 0);
    place(0, 0, mMoneyLabel2);
    place(1, 0, mMoneyField);
    place(2, 0, moneyChange).setHAlign(LayoutCell::LEFT);
    place = getPlacer(0, 2);
    place(0, 0, addButton);
#ifdef EATHENA_SUPPORT
    place(1, 0, mOkButton);
#else
    place(2, 0, mTradeButton);
    place(3, 0, cancelButton);
#endif
    Layout &layout = getLayout();
    layout.extend(0, 2, 2, 1);
    layout.setRowHeight(1, Layout::AUTO_SET);
    layout.setRowHeight(2, 0);
    layout.setColWidth(0, Layout::AUTO_SET);
    layout.setColWidth(1, Layout::AUTO_SET);

#ifdef EATHENA_SUPPORT
    mOkButton->setCaption(_("OK"));
#endif

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
    setStatus(PREPARING);
}

void TradeWindow::addItem(int id, bool own, int quantity)
{
    (own ? mMyInventory : mPartnerInventory)->addItem(id, quantity);
    setStatus(PREPARING);
}

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
    mOkButton->setCaption(_("OK"));
    mOkButton->setActionEventId("ok");
    mOkButton->setEnabled(true);
    mOkOther = false;
    mOkMe = false;
#endif
    mMoneyLabel->setCaption(strprintf(_("You get %s."), ""));
    mMoneyField->setEnabled(true);
    mMoneyField->setText("");
    setStatus(PREPARING);
}

#ifdef TMWSERV_SUPPORT

void TradeWindow::receivedOk()
{
    setStatus(ACCEPTING);
}

#else

void TradeWindow::receivedOk(bool own)
{
    if (own)
    {
        mOkMe = true;
        if (mOkOther)
        {
            mOkButton->setCaption(_("Trade"));
            mOkButton->setActionEventId("trade");
        }
    }
    else
    {
        mOkOther = true;
        if (mOkMe)
        {
            mOkButton->setCaption(_("Trade"));
            mOkButton->setActionEventId("trade");
        }
    }
}

#endif

void TradeWindow::tradeItem(Item *item, int quantity)
{
    Net::getTradeHandler()->addItem(item, quantity);
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
    Item *item = inventoryWindow->getSelectedItem();

    if (event.getId() == "add")
    {
        if (!inventoryWindow->isVisible())
            return;

        if (!item)
            return;

        if (mMyInventory->getFreeSlot() == -1)
            return;

        if (mMyInventory->contains(item))
        {
           localChatTab->chatLog(_("Failed adding item. You can not "
                                  "overlap one kind of item on the window."),
                                  BY_SERVER);
            return;
        }

        if (item->getQuantity() == 1)
        {
            tradeItem(item, 1);
        }
        else
        {
            // Choose amount of items to trade
            new ItemAmountWindow(ItemAmountWindow::TradeAdd, this, item);
        }

        setStatus(PREPARING);
    }
    else if (event.getId() == "cancel")
    {
        setVisible(false);
        reset();
        player_node->setTrading(false);

        Net::getTradeHandler()->cancel();
    }
#ifdef EATHENA_SUPPORT
    else if (event.getId() == "ok")
    {
        std::stringstream tempMoney(mMoneyField->getText());
        int tempInt;
        if (tempMoney >> tempInt)
        {
            mMoneyField->setText(toString(tempInt));

            Net::getTradeHandler()->setMoney(tempInt);
        }
        else
        {
            mMoneyField->setText("");
        }
        mMoneyField->setEnabled(false);
        Net::getTradeHandler()->confirm();
    }
#endif
    else if (event.getId() == "trade")
    {
        Net::getTradeHandler()->finish();
        setStatus(PROPOSING);
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

void TradeWindow::close()
{
    Net::getTradeHandler()->cancel();
}
