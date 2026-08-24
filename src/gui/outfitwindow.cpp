/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "outfitwindow.h"

#include "configuration.h"
#include "inventory.h"
#include "item.h"
#include "playerinfo.h"

#include "gui/widgets/button.h"
#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/outfitcontainer.h"
#include "gui/widgets/scrollarea.h"

#include "resources/itemdb.h"
#include "resources/iteminfo.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <algorithm>
#include <set>

/**
 * The number of item slots shown per row and column by default.
 */
constexpr int OUTFIT_COLUMNS = 3;
constexpr int OUTFIT_ROWS = 3;

/**
 * Values used for calculating the default window size.
 */
constexpr int SLOTS_PADDING = 3;
constexpr int LAYOUT_MARGIN = 6;

/**
 * Returns how many items of the given type can be worn at the same time.
 */
static int equipCapacity(ItemType type)
{
    // There are two ring slots
    if (type == ITEM_EQUIPMENT_RING)
        return 2;

    return 1;
}

OutfitWindow::OutfitWindow():
    Window(_("Outfits"))
{
    setWindowName("Outfits");
    setResizable(true);
    setCloseButton(true);

    mOutfitContainer = new OutfitContainer(this);

    auto scrollArea = new ScrollArea(mOutfitContainer);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mPreviousButton = new Button(_("<"), "previous", this);
    mNextButton = new Button(_(">"), "next", this);
    mCurrentLabel = new Label(strprintf(_("Outfit: %d"), 1));
    mCurrentLabel->setAlignment(gcn::Graphics::CENTER);
    mUnequipCheck = new CheckBox(_("Unequip first"));
    mEquipButton = new Button(_("Equip"), "equip", this);

    mUnequipCheck->setActionEventId("unequip");
    mUnequipCheck->addActionListener(this);

    // The size of the item slots is determined below
    scrollArea->setHeight(0);

    place(0, 0, scrollArea, 4).setPadding(SLOTS_PADDING);
    place(0, 1, mPreviousButton, 1);
    place(1, 1, mCurrentLabel, 2);
    place(3, 1, mNextButton, 1);
    place(0, 2, mUnequipCheck, 4);
    place(0, 3, mEquipButton, 4);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    load();

    // Determine the space needed by everything but the item slots
    int contentWidth = 0;
    int contentHeight = 0;
    layout.reflow(contentWidth, contentHeight);

    const int boxWidth = mOutfitContainer->getBoxWidth();
    const int boxHeight = mOutfitContainer->getBoxHeight();
    // Space around the item slots. The scroll bar is included in the width,
    // so that the number of columns does not change when it appears.
    const int slotsFrame = 2 * mOutfitContainer->getFrameSize();
    const int slotsBorder = 2 * (LAYOUT_MARGIN + SLOTS_PADDING) + slotsFrame +
                            scrollArea->getScrollbarWidth();
    const int extraHeight = contentHeight + slotsFrame + getPadding() +
                            getTitleBarHeight();

    contentWidth = std::max(contentWidth,
                            OUTFIT_COLUMNS * boxWidth + slotsBorder);

    setMinWidth(contentWidth + 2 * getPadding());
    setMinHeight(boxHeight + extraHeight);
    setDefaultSize(250, 250, getMinWidth(),
                   OUTFIT_ROWS * boxHeight + extraHeight);

    loadWindowState();

    mUnequipCheck->setSelected(mOutfits[mCurrentOutfit].unequip);
}

OutfitWindow::~OutfitWindow()
{
    save();
}

void OutfitWindow::load()
{
    for (auto &outfit : mOutfits)
    {
        outfit.items.clear();
        outfit.unequip = true;
    }

    for (auto &outfit : config.outfits)
    {
        if (outfit.index < 0 || outfit.index >= OUTFITS_COUNT)
            continue;

        auto &items = mOutfits[outfit.index].items;

        std::string buf;
        std::stringstream ss(outfit.items);

        while (ss >> buf)
        {
            // Empty slots were saved as -1 by previous versions
            if (const int itemId = atoi(buf.c_str()); itemId > 0)
                items.push_back(itemId);
        }

        mOutfits[outfit.index].unequip = outfit.unequip;
    }
}

void OutfitWindow::save()
{
    config.outfits.clear();

    for (int o = 0; o < OUTFITS_COUNT; o++)
    {
        const auto &items = mOutfits[o].items;
        if (items.empty())
            continue;

        std::string outfitStr;
        for (int item : items)
        {
            if (!outfitStr.empty())
                outfitStr += " ";
            outfitStr += toString(item);
        }

        config.outfits.push_back({ o, outfitStr, mOutfits[o].unequip });
    }
}

void OutfitWindow::logic()
{
    Window::logic();

    mOutfitContainer->setMaxItems(getItemCount());
}

int OutfitWindow::getItem(int index) const
{
    const auto &items = mOutfits[mCurrentOutfit].items;
    if (index < 0 || index >= static_cast<int>(items.size()))
        return -1;

    return items[index];
}

bool OutfitWindow::insertItem(int itemId, int index)
{
    if (itemId <= 0)
        return false;

    auto &items = mOutfits[mCurrentOutfit].items;
    const auto it = std::find(items.begin(), items.end(), itemId);

    if (it != items.end())
    {
        // The item is already in the outfit, only move it when a slot was
        // targeted
        if (index >= 0)
            moveItem(std::distance(items.begin(), it), index);
        return true;
    }

    if (index < 0 || index > static_cast<int>(items.size()))
        index = items.size();

    items.insert(items.begin() + index, itemId);
    return true;
}

void OutfitWindow::moveItem(int fromIndex, int toIndex)
{
    auto &items = mOutfits[mCurrentOutfit].items;
    const int lastIndex = static_cast<int>(items.size()) - 1;

    if (fromIndex < 0 || fromIndex > lastIndex)
        return;

    toIndex = std::clamp(toIndex, 0, lastIndex);
    if (toIndex == fromIndex)
        return;

    const int itemId = items[fromIndex];
    items.erase(items.begin() + fromIndex);
    items.insert(items.begin() + toIndex, itemId);
}

void OutfitWindow::removeItem(int index)
{
    auto &items = mOutfits[mCurrentOutfit].items;
    if (index < 0 || index >= static_cast<int>(items.size()))
        return;

    items.erase(items.begin() + index);
}

void OutfitWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "next")
    {
        if (mCurrentOutfit < (OUTFITS_COUNT - 1))
            mCurrentOutfit++;
        else
            mCurrentOutfit = 0;
        mCurrentLabel->setCaption(strprintf(_("Outfit: %d"), mCurrentOutfit + 1));
        mUnequipCheck->setSelected(mOutfits[mCurrentOutfit].unequip);
    }
    else if (event.getId() == "previous")
    {
        if (mCurrentOutfit > 0)
            mCurrentOutfit--;
        else
            mCurrentOutfit = OUTFITS_COUNT - 1;

        mCurrentLabel->setCaption(strprintf(_("Outfit: %d"), mCurrentOutfit + 1));
        mUnequipCheck->setSelected(mOutfits[mCurrentOutfit].unequip);
    }
    else if (event.getId() == "unequip")
    {
        mOutfits[mCurrentOutfit].unequip = mUnequipCheck->isSelected();
    }
    else if (event.getId() == "equip")
    {
        wearOutfit(mCurrentOutfit);
    }
}

void OutfitWindow::wearOutfit(int outfit)
{
    Inventory *inventory = PlayerInfo::getInventory();
    if (!inventory)
        return;

    const std::vector<int> items = itemsToEquip(outfit);

    if (mOutfits[outfit].unequip)
        unequip(items);

    for (int itemId : items)
    {
        Item *item = inventory->findItem(itemId);
        if (item && !item->isEquipped())
            item->doEvent(Event::DoEquip);
    }
}

std::vector<int> OutfitWindow::itemsToEquip(int outfit) const
{
    const Inventory *inventory = PlayerInfo::getInventory();
    const auto &items = mOutfits[outfit].items;

    std::vector<int> result;
    std::set<ItemType> handledTypes;

    for (size_t i = 0; i < items.size(); i++)
    {
        const ItemInfo &info = itemDb->get(items[i]);
        if (!info.equippable)
            continue;
        if (!handledTypes.insert(info.type).second)
            continue;

        // Collect the items of this outfit which use the same equipment slot
        std::vector<int> sameType;
        for (size_t j = i; j < items.size(); j++)
            if (itemDb->get(items[j]).type == info.type)
                sameType.push_back(items[j]);

        const size_t capacity = equipCapacity(info.type);
        if (sameType.size() <= capacity)
        {
            result.insert(result.end(), sameType.begin(), sameType.end());
            continue;
        }

        // Continue after the last of them which is currently equipped, so
        // that wearing the outfit again equips the next ones in line
        size_t first = 0;
        for (size_t k = 0; k < sameType.size(); k++)
        {
            const Item *item = inventory->findItem(sameType[k]);
            if (item && item->isEquipped())
                first = (k + 1) % sameType.size();
        }

        for (size_t k = 0, found = 0;
             k < sameType.size() && found < capacity; k++)
        {
            const int itemId = sameType[(first + k) % sameType.size()];
            const Item *item = inventory->findItem(itemId);
            if (item && item->getQuantity() > 0)
            {
                result.push_back(itemId);
                found++;
            }
        }
    }

    return result;
}

/**
 * Unequips all equipment which is not in the given list of items.
 */
void OutfitWindow::unequip(const std::vector<int> &exceptItems)
{
    Inventory *inventory = PlayerInfo::getInventory();

    for (int i = 0; i < inventory->getSize(); i++)
    {
        Item *item = inventory->getItem(i);
        if (!item || !item->isEquipped())
            continue;

        if (std::find(exceptItems.begin(), exceptItems.end(),
                      item->getId()) == exceptItems.end())
        {
            item->doEvent(Event::DoUnequip);
        }
    }
}

void OutfitWindow::copyOutfit(int outfit)
{
    mOutfits[mCurrentOutfit].items = mOutfits[outfit].items;
}

/**
 * Adds the item selected in the inventory window when clicking next to the
 * item slots.
 */
void OutfitWindow::mousePressed(gcn::MouseEvent &event)
{
    Window::mousePressed(event);

    if (event.getButton() == gcn::MouseEvent::LEFT && isItemSelected())
    {
        insertItem(mItemSelected, -1);
        mItemSelected = -1;
    }
}

/**
 * Handles dropping an item on the outfit window, but outside of the item
 * slots. The item is added at the end.
 */
bool OutfitWindow::handleDrop(const Drag &drag, int absX, int absY)
{
    const Item *item = drag.item.get();
    if (!item || !item->isEquippable())
        return false;

    // If an item is dragged out of a slot, let dragFinished remove it
    if (drag.source == mOutfitContainer)
        return false;

    return insertItem(item->getId(), -1);
}
