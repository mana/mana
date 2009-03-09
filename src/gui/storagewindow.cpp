/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#include <string>

#include <guichan/font.hpp>
#include <guichan/mouseinput.hpp>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "inventorywindow.h"
#include "item_amount.h"
#include "itemcontainer.h"
#include "progressbar.h"
#include "scrollarea.h"
#include "storagewindow.h"
#include "viewport.h"

#include "widgets/layout.h"

#include "../inventory.h"
#include "../item.h"
#include "../localplayer.h"

#include "../net/messageout.h"
#include "../net/network.h"
#include "../net/protocol.h"

#include "../resources/iteminfo.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"
#include "../utils/stringutils.h"

StorageWindow::StorageWindow(Network *network, int invSize):
    Window(_("Storage")),
    mNetwork(network),
    mMaxSlots(invSize),
    mItemDesc(false)
{
    setWindowName("Storage");
    setResizable(true);

    // If you adjust these defaults, don't forget to adjust the trade window's.
    setDefaultSize(115, 25, 375, 300);

    mCancelButton = new Button(_("Close"), "close", this);
    mStoreButton = new Button(_("Store"), "store", this);
    mRetrieveButton = new Button(_("Retrieve"), "retrieve", this);

    mItems = new ItemContainer(player_node->getStorage(), 1);
    mItems->addSelectionListener(this);

    mInvenScroll = new ScrollArea(mItems);
    mInvenScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mUsedSlots = toString(player_node->getStorage()->getNumberOfSlotsUsed());

    mSlotsLabel = new gcn::Label(_("Slots: "));

    mSlotsBar = new ProgressBar(1.0f, 100, 20, 225, 200, 25);

    setMinHeight(130);
    setMinWidth(mSlotsLabel->getWidth());

    place(0, 0, mSlotsLabel).setPadding(3);
    place(1, 0, mSlotsBar, 3);
    place(0, 1, mInvenScroll, 4, 4);
    place(0, 5, mCancelButton);
    place(2, 5, mStoreButton);
    place(3, 5, mRetrieveButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, mStoreButton->getHeight());

    loadWindowState();
    setLocationRelativeTo(getParent());
}

StorageWindow::~StorageWindow()
{
    delete mItems;
}

void StorageWindow::logic()
{
    if (!isVisible())
        return;

    Window::logic();

    if (mUsedSlots != toString(player_node->getStorage()->getNumberOfSlotsUsed()))
    {
        mUsedSlots = toString(player_node->getStorage()->getNumberOfSlotsUsed());

        mSlotsBar->setProgress((float)
               player_node->getStorage()->getNumberOfSlotsUsed() / mMaxSlots);

        mSlotsBar->setText(strprintf("%s/%d", mUsedSlots.c_str(), mMaxSlots));
    }
}

void StorageWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "close")
    {
        close();
    }
    else if (event.getId() == "store")
    {
        Item *item = inventoryWindow->getSelectedItem();

        if (!item)
            return;

        if (item->getQuantity() == 1)
        {
            addStore(item, 1);
        }
        else
        {
            // Choose amount of items to trade
            new ItemAmountWindow(AMOUNT_STORE_ADD, this, item);
        }
    }
    else if (event.getId() == "retrieve")
    {
        Item *item = mItems->getSelectedItem();

        if (!item)
            return;

        if (item->getQuantity() == 1)
        {
            removeStore(item, 1);
        }
        else
        {
            // Choose amount of items to trade
            new ItemAmountWindow(AMOUNT_STORE_REMOVE, this, item);
        }
    }
}

void StorageWindow::mouseClicked(gcn::MouseEvent &event)
{
    Window::mouseClicked(event);

    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Item *item = mItems->getSelectedItem();

        if (!item) {
            mRetrieveButton->setEnabled(false);
            return;
        }

        mRetrieveButton->setEnabled(true);

        /* Convert relative to the window coordinates to absolute screen
         * coordinates.
         */
        const int mx = event.getX() + getX();
        const int my = event.getY() + getY();
        viewport->showPopup(mx, my, item);
    }
}

Item* StorageWindow::getSelectedItem() const
{
    return mItems->getSelectedItem();
}

void StorageWindow::addStore(Item* item, int ammount)
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_MOVE_TO_STORAGE);
    outMsg.writeInt16(item->getInvIndex());
    outMsg.writeInt32(ammount);
}

void StorageWindow::removeStore(Item* item, int ammount)
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CSMG_MOVE_FROM_STORAGE);
    outMsg.writeInt16(item->getInvIndex());
    outMsg.writeInt32(ammount);
}

void StorageWindow::close()
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_CLOSE_STORAGE);
}
