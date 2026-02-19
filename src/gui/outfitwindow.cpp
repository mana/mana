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
#include "equipment.h"
#include "graphics.h"
#include "inventory.h"
#include "item.h"
#include "playerinfo.h"

#include "gui/gui.h"
#include "gui/widgets/button.h"
#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"

#include "resources/image.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

OutfitWindow::OutfitWindow():
    Window(_("Outfits"))
{
    setWindowName("Outfits");
    setResizable(true);
    setCloseButton(true);
    setDefaultSize(250, 250, 118, 180);

    mPreviousButton = new Button(_("<"), "previous", this);
    mNextButton = new Button(_(">"), "next", this);
    mCurrentLabel = new Label(strprintf(_("Outfit: %d"), 1));
    mCurrentLabel->setAlignment(gcn::Graphics::CENTER);
    mUnequipCheck = new CheckBox(_("Unequip first"));

    mUnequipCheck->setActionEventId("unequip");
    mUnequipCheck->addActionListener(this);

    place(0, 3, mPreviousButton, 1);
    place(1, 3, mCurrentLabel, 2);
    place(3, 3, mNextButton, 1);
    place(0, 4, mUnequipCheck, 4);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);
    layout.setColWidth(4, Layout::CENTER);

    loadWindowState();

    load();

    mUnequipCheck->setSelected(mOutfits[mCurrentOutfit].unequip);
}

OutfitWindow::~OutfitWindow()
{
    save();
}

void OutfitWindow::load()
{
    for (auto &mOutfit : mOutfits)
        memset(mOutfit.items, -1, sizeof(mOutfit.items));

    for (auto &outfit : config.outfits)
    {
        if (outfit.index < 0 || outfit.index >= OUTFITS_COUNT)
            continue;

        std::string buf;
        std::stringstream ss(outfit.items);

        for (int i = 0; (ss >> buf) && i < OUTFIT_ITEM_COUNT; i++)
            mOutfits[outfit.index].items[i] = atoi(buf.c_str());

        mOutfits[outfit.index].unequip = outfit.unequip;
    }
}

void OutfitWindow::save()
{
    config.outfits.clear();

    std::string outfitStr;
    for (int o = 0; o < OUTFITS_COUNT; o++)
    {
        bool emptyOutfit = true;

        for (int item : mOutfits[o].items)
        {
            if (!outfitStr.empty())
                outfitStr += " ";

            outfitStr += item ? toString(item) : toString(-1);
            emptyOutfit &= item <= 0;
        }

        if (!emptyOutfit)
            config.outfits.push_back({ o, outfitStr, mOutfits[o].unequip });

        outfitStr.clear();
    }
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
}

void OutfitWindow::wearOutfit(int outfit)
{
    if (mOutfits[outfit].unequip)
        unequipNotInOutfit(outfit);

    for (int i : mOutfits[outfit].items)
    {
        Item *item = PlayerInfo::getInventory()->findItem(i);
        if (item && !item->isEquipped() && item->getQuantity())
        {
            if (item->isEquippable())
                item->doEvent(Event::DoEquip);
        }
    }
}

void OutfitWindow::copyOutfit(int outfit)
{
    for (int i = 0; i < OUTFIT_ITEM_COUNT; i++)
        mOutfits[mCurrentOutfit].items[i] = mOutfits[outfit].items[i];
}

bool OutfitWindow::addItemToCurrentOutfit(int itemId, int targetIndex)
{
    if (itemId <= 0)
        return false;

    int existingIndex = -1;
    int firstFreeIndex = -1;
    auto &items = mOutfits[mCurrentOutfit].items;

    for (int i = 0; i < OUTFIT_ITEM_COUNT; ++i)
    {
        if (items[i] == itemId)
            existingIndex = i;
        else if (firstFreeIndex == -1 && items[i] < 0)
            firstFreeIndex = i;
    }

    if (targetIndex == -1)
    {
        if (existingIndex != -1)
            return true;
        if (firstFreeIndex == -1)
            return false;

        targetIndex = firstFreeIndex;
    }

    if (existingIndex != -1)
        std::swap(items[existingIndex], items[targetIndex]);
    else
        items[targetIndex] = itemId;

    return true;
}

/**
 * Handles dropping an item onto the outfit window.
 */
bool OutfitWindow::handleDrop(const Drag &drag, int absX, int absY)
{
    const Item *item = drag.item.get();
    if (!item)
        return false;

    int widgetX = 0;
    int widgetY = 0;
    getAbsolutePosition(widgetX, widgetY);

    const int targetIndex = getIndexFromGrid(absX - widgetX, absY - widgetY);

    // If item is dragged out of a slot, let dragFinished remove it
    if (drag.source == this && targetIndex == -1)
        return false;

    if (!item->isEquippable())
        return false;

    return addItemToCurrentOutfit(item->getId(), targetIndex);
}

void OutfitWindow::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);
    auto *g = static_cast<Graphics*>(graphics);
    const auto *drag = gui->getActiveDrag();
    const auto *inventory = PlayerInfo::getInventory();
    const auto &currentOutfit = mOutfits[mCurrentOutfit];

    for (int i = 0; i < OUTFIT_ITEM_COUNT; i++)
    {
        const int itemX = 10 + (i % mGridWidth) * mBoxWidth;
        const int itemY = 25 + (i / mGridWidth) * mBoxHeight;

        graphics->setColor(gcn::Color(0, 0, 0, 64));
        graphics->drawRectangle(gcn::Rectangle(itemX, itemY,
                                               ITEM_ICON_SIZE,
                                               ITEM_ICON_SIZE));
        graphics->setColor(gcn::Color(255, 255, 255, 32));
        graphics->fillRectangle(gcn::Rectangle(itemX, itemY,
                                               ITEM_ICON_SIZE,
                                               ITEM_ICON_SIZE));

        if (currentOutfit.items[i] < 0)
            continue;

        if (Item *item = inventory->findItem(currentOutfit.items[i]))
        {
            // Draw item icon.
            if (Image *image = item->getImage())
            {
                const bool isDragged =
                        drag &&
                        drag->source == this &&
                        drag->sourceIndex == i;
                image->setAlpha(isDragged ? 0.5f : 1.0f);
                g->drawImage(image, itemX, itemY);
            }
        }
    }
}

void OutfitWindow::mouseDragged(gcn::MouseEvent &event)
{
    Window::mouseDragged(event);

    if (mMoved)
        return;

    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (!gui->getActiveDrag() && mClickedIndex != -1)
        {
            const int itemId = mOutfits[mCurrentOutfit].items[mClickedIndex];
            if (itemId <= 0)
                return;

            if (Item *item = PlayerInfo::getInventory()->findItem(itemId))
                gui->startDrag(Drag::fromOutfit(item, this, mClickedIndex));
        }
    }
}

void OutfitWindow::mousePressed(gcn::MouseEvent &event)
{
    Window::mousePressed(event);

    mClickedIndex = getIndexFromGrid(event.getX(), event.getY());
    if (mClickedIndex == -1)
        return;

    if (event.getButton() == gcn::MouseEvent::LEFT)
        mMoved = false;         // prevent window drag

    // Stores the selected item if there is one.
    if (isItemSelected())
    {
        mOutfits[mCurrentOutfit].items[mClickedIndex] = mItemSelected;
        mItemSelected = -1;
    }
}

void OutfitWindow::mouseReleased(gcn::MouseEvent &event)
{
    Window::mouseReleased(event);

    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        mItemSelected = -1;
        mClickedIndex = -1;
    }
}

void OutfitWindow::dragFinished(const Drag &drag, DragResult result)
{
    if (result == DragResult::Ignored &&
        drag.source == this &&
        drag.sourceIndex >= 0 &&
        drag.sourceIndex < OUTFIT_ITEM_COUNT)
    {
        mOutfits[mCurrentOutfit].items[drag.sourceIndex] = -1;
    }

    mClickedIndex = -1;
}

int OutfitWindow::getIndexFromGrid(int pointX, int pointY) const
{
    const gcn::Rectangle tRect = gcn::Rectangle(
        10, 25, 10 + mGridWidth * mBoxWidth, 25 + mGridHeight * mBoxHeight);
    if (!tRect.isPointInRect(pointX, pointY))
        return -1;
    const int index = (((pointY - 25) / mBoxHeight) * mGridWidth) +
        (pointX - 10) / mBoxWidth;
    if (index >= OUTFIT_ITEM_COUNT)
        return -1;
    return index;
}

void OutfitWindow::unequipNotInOutfit(int outfit)
{
    Inventory *inventory = PlayerInfo::getInventory();
    if (!inventory)
        return;

    for (int i = 0; i < inventory->getSize(); i++)
    {
        if (inventory->getItem(i) && inventory->getItem(i)->isEquipped())
        {
            bool found = false;
            for (int item : mOutfits[outfit].items)
            {
                if (inventory->getItem(i)->getId() == item)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                if (Item *item = inventory->getItem(i))
                    item->doEvent(Event::DoUnequip);
            }
        }
    }
}
