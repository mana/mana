/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/tradewindow.h"

#include "event.h"
#include "inventory.h"
#include "item.h"
#include "playerinfo.h"
#include "units.h"

#include "gui/inventorywindow.h"
#include "gui/itemamountwindow.h"
#include "gui/setup.h"

#include "gui/widgets/button.h"
#include "gui/widgets/itemcontainer.h"
#include "gui/widgets/label.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/textfield.h"
#include "gui/widgets/layout.h"

#include "net/net.h"
#include "net/tradehandler.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>

#define CAPTION_PROPOSE _("Propose trade")
#define CAPTION_CONFIRMED _("Confirmed. Waiting...")
#define CAPTION_ACCEPT _("Agree trade")
#define CAPTION_ACCEPTED _("Agreed. Waiting...")

TradeWindow::TradeWindow():
    Window(_("Trade: You")),
    mMyInventory(new Inventory(Inventory::TRADE)),
    mPartnerInventory(new Inventory(Inventory::TRADE)),
    mStatus(PROPOSING)
{
    setWindowName("Trade");
    setResizable(true);
    setCloseButton(true);
    setDefaultSize(386, 180, WindowAlignment::Center);
    setMinWidth(386);
    setMinHeight(180);
    setupWindow->registerWindowForReset(this);

    std::string longestName = getFont()->getWidth(_("OK")) >
                                   getFont()->getWidth(_("Trade")) ?
                                   _("OK") : _("Trade");

    mAddButton = new Button(_("Add"), "add", this);
    mOkButton = new Button("", "", this); // Will be filled in later

    int width = std::max(mOkButton->getFont()->getWidth(CAPTION_PROPOSE),
                         mOkButton->getFont()->getWidth(CAPTION_CONFIRMED));
    width = std::max(width, mOkButton->getFont()->getWidth(CAPTION_ACCEPT));
    width = std::max(width, mOkButton->getFont()->getWidth(CAPTION_ACCEPTED));

    mOkButton->setWidth(8 + width);

    mMyItemContainer = new ItemContainer(mMyInventory.get());
    mMyItemContainer->addSelectionListener(this);

    auto *myScroll = new ScrollArea(mMyItemContainer);
    myScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mPartnerItemContainer = new ItemContainer(mPartnerInventory.get());
    mPartnerItemContainer->addSelectionListener(this);

    auto *partnerScroll = new ScrollArea(mPartnerItemContainer);
    partnerScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mMoneyLabel = new Label(strprintf(_("You get %s"), ""));
    gcn::Label *mMoneyLabel2 = new Label(_("You give:"));

    mMoneyField = new TextField;
    mMoneyField->setWidth(40);
    mMoneyChangeButton = new Button(_("Change"), "money", this);

    place(1, 0, mMoneyLabel);
    place(0, 1, myScroll).setPadding(3);
    place(1, 1, partnerScroll).setPadding(3);
    ContainerPlacer place;
    place = getPlacer(0, 0);
    place(0, 0, mMoneyLabel2);
    place(1, 0, mMoneyField);
    place(2, 0, mMoneyChangeButton).setHAlign(LayoutCell::LEFT);
    place = getPlacer(0, 2);
    place(0, 0, mAddButton);
    place(1, 0, mOkButton);
    Layout &layout = getLayout();
    layout.extend(0, 2, 2, 1);
    layout.setRowHeight(1, Layout::AUTO_SET);
    layout.setRowHeight(2, 0);
    layout.setColWidth(0, Layout::AUTO_SET);
    layout.setColWidth(1, Layout::AUTO_SET);

    loadWindowState();

    reset();
}

TradeWindow::~TradeWindow() = default;

void TradeWindow::setMoney(int amount)
{
    mMoneyLabel->setCaption(strprintf(_("You get %s"),
                                       Units::formatCurrency(amount).c_str()));
    mMoneyLabel->adjustSize();
}

void TradeWindow::addItem(int id, bool own, int quantity)
{
    if (own)
        mMyInventory->addItem(id, quantity);
    else
        mPartnerInventory->addItem(id, quantity);
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
    mOkOther = false;
    mOkMe = false;
    setMoney(0);
    mMoneyField->setEnabled(true);
    mMoneyField->setText(std::string());
    mAddButton->setEnabled(true);
    mMoneyChangeButton->setEnabled(true);
    setStatus(PREPARING);
}

void TradeWindow::receivedOk(bool own)
{
    if (own)
        mOkMe = true;
    else
        mOkOther = true;

    if (mOkMe && mOkOther)
    {
        //mOkMe = false;
        //mOkOther = false;
        setStatus(ACCEPTING);
    }
}

void TradeWindow::valueChanged(const gcn::SelectionEvent &event)
{
    /* If an item is selected in one container, make sure no item is selected
     * in the other container.
     */
    if (event.getSource() == mMyItemContainer &&
            mMyItemContainer->getSelectedItem())
        mPartnerItemContainer->selectNone();
    else if (mPartnerItemContainer->getSelectedItem())
        mMyItemContainer->selectNone();
}

void TradeWindow::setStatus(Status s)
{
    if (s == mStatus)
        return;
    mStatus = s;

    switch (s)
    {
        case PREPARING:
            mOkButton->setCaption(CAPTION_PROPOSE);
            mOkButton->setActionEventId("ok");
            break;
        case PROPOSING:
            mOkButton->setCaption(CAPTION_CONFIRMED);
            mOkButton->setActionEventId(std::string());
            break;
        case ACCEPTING:
            mOkButton->setCaption(CAPTION_ACCEPT);
            mOkButton->setActionEventId("trade");
            break;
        case ACCEPTED:
            mOkButton->setCaption(CAPTION_ACCEPTED);
            mOkButton->setActionEventId(std::string());
            break;
        default:
            break;
    }

    mOkButton->setEnabled((s != PROPOSING && s != ACCEPTED));
}

void TradeWindow::action(const gcn::ActionEvent &event)
{
    Item *item = inventoryWindow->getSelectedItem();

    if (event.getId() == "add")
    {
        if (mStatus != PREPARING)
            return;

        if (!inventoryWindow->isVisible())
        {
            inventoryWindow->setVisible(true);
            return;
        }

        if (!item)
            return;

        if (mMyInventory->getFreeSlot() == -1)
            return;

        if (mMyInventory->contains(item))
        {
            serverNotice(_("Failed adding item. You cannot "
                           "overlap one kind of item on the window."));
            return;
        }

        // Choose amount of items to trade
        ItemAmountWindow::showWindow(ItemAmountWindow::TradeAdd, this, item);

        setStatus(PREPARING);
    }
    else if (event.getId() == "cancel")
    {
        setVisible(false);
        reset();

        Net::getTradeHandler()->cancel();
    }
    else if (event.getId() == "ok")
    {
        mMoneyField->setEnabled(false);
        mAddButton->setEnabled(false);
        mMoneyChangeButton->setEnabled(false);
        receivedOk(true);
        setStatus(PROPOSING);
        Net::getTradeHandler()->confirm();
    }
    else if (event.getId() == "trade")
    {
        receivedOk(true);
        setStatus(ACCEPTED);
        Net::getTradeHandler()->finish();
    }
    else if (event.getId() == "money")
    {
        if (mStatus != PREPARING) 
            return;

        int v = atoi(mMoneyField->getText().c_str());
        int curMoney = PlayerInfo::getAttribute(MONEY);
        if (v > curMoney)
        {
            serverNotice(_("You don't have enough money."));
            v = curMoney;
        }
        Net::getTradeHandler()->setMoney(v);
        mMoneyField->setText(strprintf("%d", v));
    }
}

void TradeWindow::close()
{
    Net::getTradeHandler()->cancel();
}
