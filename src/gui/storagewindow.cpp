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

#include "gui/storagewindow.h"

#include "gui/inventorywindow.h"
#include "gui/itemamount.h"
#include "gui/itemcontainer.h"
#include "gui/viewport.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/scrollarea.h"

#include "inventory.h"
#include "item.h"
#include "localplayer.h"
#include "units.h"

#include "net/net.h"
#include "net/ea/inventoryhandler.h"

#include "resources/iteminfo.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>
#include <guichan/mouseinput.hpp>

#include <string>

StorageWindow::StorageWindow(int invSize):
    Window(_("Storage")),
    mMaxSlots(invSize),
    mItemDesc(false)
{
    setWindowName("Storage");
    setResizable(true);
    setCloseButton(true);

    // If you adjust these defaults, don't forget to adjust the trade window's.
    setDefaultSize(375, 300, ImageRect::CENTER);

    mStoreButton = new Button(_("Store"), "store", this);
    mRetrieveButton = new Button(_("Retrieve"), "retrieve", this);

    mCloseButton = new Button(_("Close"), "close", this);

    mItems = new ItemContainer(player_node->getStorage(), true);
    mItems->addSelectionListener(this);

    gcn::ScrollArea *invenScroll = new ScrollArea(mItems);
    invenScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mUsedSlots = player_node->getStorage()->getNumberOfSlotsUsed();

    mSlotsLabel = new Label(_("Slots:"));

    mSlotsBar = new ProgressBar(0.0f, 100, 20, gcn::Color(225, 200, 25));
    mSlotsBar->setText(strprintf("%d/%d", mUsedSlots, mMaxSlots));
    mSlotsBar->setProgress((float) mUsedSlots / mMaxSlots);

    setMinHeight(130);
    setMinWidth(200);

    place(0, 0, mSlotsLabel).setPadding(3);
    place(1, 0, mSlotsBar, 3);
    place(0, 1, invenScroll, 4, 4);
    place(0, 5, mStoreButton);
    place(1, 5, mRetrieveButton);
    place(3, 5, mCloseButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, mStoreButton->getHeight());

    loadWindowState();
}

StorageWindow::~StorageWindow()
{
}

void StorageWindow::logic()
{
    if (!isVisible())
        return;

    Window::logic();

    const int usedSlots = player_node->getStorage()->getNumberOfSlotsUsed();

    if (mUsedSlots != usedSlots)
    {
        mUsedSlots = usedSlots;

        mSlotsBar->setProgress((float) mUsedSlots / mMaxSlots);

        mSlotsBar->setText(strprintf("%d/%d", mUsedSlots, mMaxSlots));
    }
}

void StorageWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "store")
    {
        if (!inventoryWindow->isVisible()) return;

        Item *item = inventoryWindow->getSelectedItem();

        if (!item)
            return;

        ItemAmountWindow::showWindow(ItemAmountWindow::StoreAdd, this, item);
    }
    else if (event.getId() == "retrieve")
    {
        Item *item = mItems->getSelectedItem();

        if (!item)
            return;

        ItemAmountWindow::showWindow(ItemAmountWindow::StoreRemove, this,
                                     item);
    }
    else if (event.getId() == "close")
    {
        close();
    }
}

void StorageWindow::mouseClicked(gcn::MouseEvent &event)
{
    Window::mouseClicked(event);

    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Item *item = mItems->getSelectedItem();

        if (!item)
        {
            mRetrieveButton->setEnabled(false);
            return;
        }

        mRetrieveButton->setEnabled(true);

        /* Convert relative to the window coordinates to absolute screen
         * coordinates.
         */
        const int mx = event.getX() + getX();
        const int my = event.getY() + getY();
        viewport->showPopup(mx, my, item, false);
    }
}

Item *StorageWindow::getSelectedItem() const
{
    return mItems->getSelectedItem();
}

void StorageWindow::addStore(Item *item, int amount)
{
    Net::getInventoryHandler()->moveItem(Net::InventoryHandler::INVENTORY,
                                         item->getInvIndex(), amount,
                                         Net::InventoryHandler::STORAGE);
}

void StorageWindow::removeStore(Item *item, int amount)
{
    Net::getInventoryHandler()->moveItem(Net::InventoryHandler::STORAGE,
                                         item->getInvIndex(), amount,
                                         Net::InventoryHandler::INVENTORY);
}

void StorageWindow::close()
{
    Net::getInventoryHandler()->closeStorage();
}
