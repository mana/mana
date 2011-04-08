/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "gui/inventorywindow.h"

#include "inventory.h"
#include "item.h"
#include "units.h"
#include "keyboardconfig.h"
#include "playerinfo.h"

#include "gui/itemamount.h"
#include "gui/setup.h"
#include "gui/sdlinput.h"
#include "gui/viewport.h"

#include "gui/widgets/button.h"
#include "gui/widgets/itemcontainer.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/scrollarea.h"

#include "net/inventoryhandler.h"
#include "net/net.h"

#include "resources/iteminfo.h"
#include "resources/theme.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>
#include <guichan/mouseinput.hpp>

#include <string>

InventoryWindow::WindowList InventoryWindow::instances;

InventoryWindow::InventoryWindow(Inventory *inventory):
    Window(inventory->isMainInventory() ? _("Inventory") : _("Storage")),
    mInventory(inventory),
    mFilterText(new TextField),
    mSplit(false)
{
    listen(Mana::Event::AttributesChannel);

    setWindowName(isMainInventory() ? "Inventory" : "Storage");
    setupWindow->registerWindowForReset(this);
    setResizable(true);
    setCloseButton(true);
    setSaveVisible(true);

    setDefaultSize(387, 307, ImageRect::CENTER);
    setMinWidth(316);
    setMinHeight(179);
    addKeyListener(this);

    mFilterText->setWidth(150);
    mFilterText->addKeyListener(this);

    mItems = new ItemContainer(mInventory);
    mItems->addSelectionListener(this);

    gcn::ScrollArea *invenScroll = new ScrollArea(mItems);
    invenScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mSlotsLabel = new Label(_("Slots:"));
    mFilterLabel = new Label(_("Search:"));
    mSlotsBar = new ProgressBar(0.0f, 100, 20, Theme::PROG_INVY_SLOTS);

    if (isMainInventory())
    {
        std::string equip = _("Equip");
        std::string use = _("Use");
        std::string unequip = _("Unequip");

        std::string longestUseString = getFont()->getWidth(equip) >
                                       getFont()->getWidth(use) ? equip : use;

        if (getFont()->getWidth(longestUseString) <
            getFont()->getWidth(unequip))
        {
            longestUseString = unequip;
        }

        mEquipButton = new Button(_("Equip"), "equip", this);
        mUseButton = new Button(_("Activate"), "activate", this);
        mDropButton = new Button(_("Drop..."), "drop", this);
        mSplitButton = new Button(_("Split"), "split", this);
        mOutfitButton = new Button(_("Outfits"), "outfit", this);

        mWeightLabel = new Label(_("Weight:"));
        mWeightBar = new ProgressBar(0.0f, 100, 20, Theme::PROG_WEIGHT);

        place(0, 0, mWeightLabel).setPadding(3);
        place(1, 0, mWeightBar, 3);
        place(4, 0, mSlotsLabel).setPadding(3);
        place(5, 0, mSlotsBar, 3);
        place(0, 1, mFilterLabel, 4);
        place(1, 1, mFilterText, 4);
        place(0, 2, invenScroll, 8).setPadding(3);
        place(0, 3, mUseButton);
        place(1, 3, mEquipButton);
        place(2, 3, mDropButton);
        place(3, 3, mSplitButton);
        place(7, 3, mOutfitButton);

        updateWeight();
    }
    else
    {
        mStoreButton = new Button(_("Store"), "store", this);
        mRetrieveButton = new Button(_("Retrieve"), "retrieve", this);

        place(0, 0, mSlotsLabel).setPadding(3);
        place(1, 0, mSlotsBar, 3);
        place(0, 1, mFilterLabel).setPadding(3);
        place(1, 1, mFilterText, 3);
        place(0, 2, invenScroll, 4, 4);
        place(0, 6, mStoreButton);
        place(1, 6, mRetrieveButton);
    }

    Layout &layout = getLayout();
    layout.setRowHeight(2, Layout::AUTO_SET);

    mInventory->addInventoyListener(this);

    instances.push_back(this);

    loadWindowState();
    slotsChanged(mInventory);

    if (!isMainInventory())
    {
        setVisible(true);
        PlayerInfo::setStorageCount(PlayerInfo::getStorageCount() + 1);
    }
}

InventoryWindow::~InventoryWindow()
{
    instances.remove(this);
    mInventory->removeInventoyListener(this);

    if (!isMainInventory())
        PlayerInfo::setStorageCount(PlayerInfo::getStorageCount() - 1);
}

void InventoryWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "outfit")
    {
        extern Window *outfitWindow;
        outfitWindow->setVisible(!outfitWindow->isVisible());
        if (outfitWindow->isVisible())
        {
            outfitWindow->requestMoveToTop();
        }
    }
    else if (event.getId() == "store")
    {
        if (!inventoryWindow->isVisible()) return;

        Item *item = inventoryWindow->getSelectedItem();

        if (!item)
            return;

        ItemAmountWindow::showWindow(ItemAmountWindow::StoreAdd, this, item);
    }

    Item *item = mItems->getSelectedItem();

    if (!item)
        return;

    if (event.getId() == "activate")
        item->doEvent(Mana::Event::DoUse);
    else if (event.getId() == "equip")
    {
        if (item->isEquippable())
        {
            if (item->isEquipped())
                item->doEvent(Mana::Event::DoUnequip);
            else
                item->doEvent(Mana::Event::DoEquip);
        }
        else
        {
            item->doEvent(Mana::Event::DoUse);
        }
    }
    else if (event.getId() == "drop")
    {
        ItemAmountWindow::showWindow(ItemAmountWindow::ItemDrop, this, item);
    }
    else if (event.getId() == "split")
    {
        ItemAmountWindow::showWindow(ItemAmountWindow::ItemSplit, this, item,
                                 (item->getQuantity() - 1));
    }
    else if (event.getId() == "retrieve")
    {
        Item *item = mItems->getSelectedItem();

        if (!item)
            return;

        ItemAmountWindow::showWindow(ItemAmountWindow::StoreRemove, this,
                                     item);
    }
}

Item *InventoryWindow::getSelectedItem() const
{
    return mItems->getSelectedItem();
}

void InventoryWindow::mouseClicked(gcn::MouseEvent &event)
{
    Window::mouseClicked(event);

    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Item *item = mItems->getSelectedItem();

        if (!item)
            return;

        /* Convert relative to the window coordinates to absolute screen
         * coordinates.
         */
        const int mx = event.getX() + getX();
        const int my = event.getY() + getY();
        viewport->showPopup(this, mx, my, item, isMainInventory());
    }

    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (instances.size() > 1 && keyboard.isKeyActive(keyboard.KEY_EMOTE))
        {
            Item *item = mItems->getSelectedItem();

            if(!item)
                return;
            if (mInventory->isMainInventory())
            {
                Mana::Event event(Mana::Event::DoMove);
                event.setItem("item", item);
                event.setInt("amount", item->getQuantity());
                event.setInt("source", Inventory::INVENTORY);
                event.setInt("destination", Inventory::STORAGE);
                event.trigger(Mana::Event::ItemChannel);
            }
            else
            {
                Mana::Event event(Mana::Event::DoMove);
                event.setItem("item", item);
                event.setInt("amount", item->getQuantity());
                event.setInt("source", Inventory::STORAGE);
                event.setInt("destination", Inventory::INVENTORY);
                event.trigger(Mana::Event::ItemChannel);
            }
        }
    }
}

void InventoryWindow::keyPressed(gcn::KeyEvent &event)
{
    switch (event.getKey().getValue())
    {
        case Key::LEFT_SHIFT:
        case Key::RIGHT_SHIFT:
            mSplit = true;
            break;
    }
}

void InventoryWindow::keyReleased(gcn::KeyEvent &event)
{
    if (isInputFocused())
    {
        mItems->setFilter(mFilterText->getText());
        return;
    }

    switch (event.getKey().getValue())
    {
        case Key::LEFT_SHIFT:
        case Key::RIGHT_SHIFT:
            mSplit = false;
            break;
    }
}

void InventoryWindow::valueChanged(const gcn::SelectionEvent &event)
{
    if (!mInventory->isMainInventory())
        return;

    Item *item = mItems->getSelectedItem();

    if (mSplit && Net::getInventoryHandler()->
        canSplit(mItems->getSelectedItem()) && item)
    {
        ItemAmountWindow::showWindow(ItemAmountWindow::ItemSplit, this, item,
                                     (item->getQuantity() - 1));
    }

    updateButtons();
}

void InventoryWindow::updateButtons()
{
    Item *item = mItems->getSelectedItem();

    if (!item || item->getQuantity() == 0)
    {
        mUseButton->setEnabled(false);
        mEquipButton->setEnabled(false);
        mDropButton->setEnabled(false);
        mSplitButton->setEnabled(false);

        return;
    }

    mDropButton->setEnabled(true);

    if (item->getInfo().getEquippable())
    {
        if (item->isEquipped())
            mEquipButton->setCaption(_("Unequip"));
        else
            mEquipButton->setCaption(_("Equip"));
        mEquipButton->setEnabled(true);
    }
    else
        mEquipButton->setEnabled(false);

    mUseButton->setEnabled(item->getInfo().getActivatable());

    if (item->getQuantity() > 1)
        mDropButton->setCaption(_("Drop..."));
    else
        mDropButton->setCaption(_("Drop"));

    if (Net::getInventoryHandler()->canSplit(item))
        mSplitButton->setEnabled(true);
    else
        mSplitButton->setEnabled(false);
}

void InventoryWindow::setSplitAllowed(bool allowed)
{
    mSplitButton->setVisible(allowed);
}

void InventoryWindow::close()
{
    if (this == inventoryWindow)
    {
        setVisible(false);
    }
    else
    {
        Mana::Event event(Mana::Event::DoCloseInventory);
        event.setInt("type", mInventory->getType());
        event.trigger(Mana::Event::ItemChannel);
        scheduleDelete();
    }
}

void InventoryWindow::event(Mana::Event::Channel channel, const Mana::Event &event)
{
    if (event.getType() == Mana::Event::UpdateAttribute)
    {
        int id = event.getInt("id");
        if (id == TOTAL_WEIGHT ||
            id == MAX_WEIGHT)
        {
            updateWeight();
        }
    }
}

void InventoryWindow::updateWeight()
{
    if (!isMainInventory())
        return;

    int total = PlayerInfo::getAttribute(TOTAL_WEIGHT);
    int max = PlayerInfo::getAttribute(MAX_WEIGHT);

    if (max <= 0)
        return;

    // Adjust progress bar
    mWeightBar->setProgress((float) total / max);
    mWeightBar->setText(strprintf("%s/%s", Units::formatWeight(total).c_str(),
                                  Units::formatWeight(max).c_str()));
}

bool InventoryWindow::isInputFocused() const
{
    return mFilterText->isFocused();
}

bool InventoryWindow::isAnyInputFocused()
{
    WindowList::iterator it = instances.begin();
    WindowList::iterator it_end = instances.end();

    for (; it != it_end; it++)
    {
        if ((*it)->isInputFocused())
        {
            return true;
        }
    }

    return false;
}

void InventoryWindow::slotsChanged(Inventory* inventory)
{
    if (inventory == mInventory)
    {
        const int usedSlots = mInventory->getNumberOfSlotsUsed();
        const int maxSlots = mInventory->getSize();

        mSlotsBar->setProgress((float) usedSlots / maxSlots);

        mSlotsBar->setText(strprintf("%d/%d", usedSlots, maxSlots));
    }
}
