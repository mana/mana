/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
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

#include "outfitwindow.h"

#include "configuration.h"
#include "equipment.h"
#include "graphics.h"
#include "inventory.h"
#include "item.h"
#include "playerinfo.h"

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
    mUnequipCheck = new CheckBox(_("Unequip first"),
                                 config.getValue("OutfitUnequip0", true));

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
}

OutfitWindow::~OutfitWindow()
{
    save();
}

void OutfitWindow::load()
{
    memset(mItems, -1, sizeof(mItems));
    for (int o = 0; o < OUTFITS_COUNT; o++)
    {
        std::string outfit = config.getValue("Outfit" + toString(o), "-1");
        std::string buf;
        std::stringstream ss(outfit);

        for (size_t i = 0; (ss >> buf) && i < OUTFIT_ITEM_COUNT; i++)
        {
            mItems[o][i] = atoi(buf.c_str());
        }

        mItemsUnequip[o] = config.getValue("OutfitUnequip" + toString(o), true);
    }
}

void OutfitWindow::save()
{
    std::string outfitStr;
    for (int o = 0; o < OUTFITS_COUNT; o++)
    {
        for (int i = 0; i < OUTFIT_ITEM_COUNT; i++)
        {
            outfitStr += mItems[o][i] ? toString(mItems[o][i]) : toString(-1);
            if (i <8) outfitStr += " ";
        }
        config.setValue("Outfit" + toString(o), outfitStr);
        config.setValue("OutfitUnequip" + toString(o), mItemsUnequip[o]);
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
        mUnequipCheck->setSelected(mItemsUnequip[mCurrentOutfit]);
    }
    else if (event.getId() == "previous")
    {
        if (mCurrentOutfit > 0)
        {
            mCurrentOutfit--;
        }
        else
        {
            mCurrentOutfit = OUTFITS_COUNT - 1;
        }
        mCurrentLabel->setCaption(strprintf(_("Outfit: %d"), mCurrentOutfit + 1));
        mUnequipCheck->setSelected(mItemsUnequip[mCurrentOutfit]);
    }
    else if (event.getId() == "unequip")
    {
        mItemsUnequip[mCurrentOutfit] = mUnequipCheck->isSelected();
    }
}

void OutfitWindow::wearOutfit(int outfit)
{
    if (mItemsUnequip[outfit])
        unequipNotInOutfit(outfit);

    Item *item;
    for (int i = 0; i < OUTFIT_ITEM_COUNT; i++)
    {
        item = PlayerInfo::getInventory()->findItem(mItems[outfit][i]);
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
    {
        mItems[mCurrentOutfit][i] = mItems[outfit][i];
    }
}

void OutfitWindow::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);
    auto *g = static_cast<Graphics*>(graphics);

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

        if (mItems[mCurrentOutfit][i] < 0)
        {
            continue;
        }

        Item *item =
               PlayerInfo::getInventory()->findItem(mItems[mCurrentOutfit][i]);
        if (item)
         {
            // Draw item icon.
            if (Image *image = item->getImage())
            {
                g->drawImage(image, itemX, itemY);
            }
        }
    }
    if (mItemMoved)
    {
        // Draw the item image being dragged by the cursor.
        if (Image *image = mItemMoved->getImage())
        {
            const int tPosX = mCursorPosX - (image->getWidth() / 2);
            const int tPosY = mCursorPosY - (image->getHeight() / 2);

            g->drawImage(image, tPosX, tPosY);
        }
    }
}


void OutfitWindow::mouseDragged(gcn::MouseEvent &event)
{
    Window::mouseDragged(event);
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (!mItemMoved && mItemClicked)
        {
            const int index = getIndexFromGrid(event.getX(), event.getY());
            if (index == -1)
                return;
            const int itemId = mItems[mCurrentOutfit][index];
            if (itemId < 0)
                return;
            Item *item = PlayerInfo::getInventory()->findItem(itemId);
            if (item)
            {
                mItemMoved = item;
                mItems[mCurrentOutfit][index] = -1;
            }
        }
        if (mItemMoved)
        {
            mCursorPosX = event.getX();
            mCursorPosY = event.getY();
        }
    }
}

void OutfitWindow::mousePressed(gcn::MouseEvent &event)
{
    Window::mousePressed(event);
    const int index = getIndexFromGrid(event.getX(), event.getY());
    if (index == -1)
        return;

    // Stores the selected item if there is one.
    if (isItemSelected())
    {
        mItems[mCurrentOutfit][index] = mItemSelected;
        mItemSelected = -1;
    }
    else if (mItems[mCurrentOutfit][index])
    {
        mItemClicked = true;
    }
}

void OutfitWindow::mouseReleased(gcn::MouseEvent &event)
{
    Window::mouseReleased(event);
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (isItemSelected())
        {
            mItemSelected = -1;
        }
        const int index = getIndexFromGrid(event.getX(), event.getY());
        if (index == -1)
        {
            mItemMoved = nullptr;
            return;
        }
        if (mItemMoved)
        {
            mItems[mCurrentOutfit][index] = mItemMoved->getId();
            mItemMoved = nullptr;
        }
        if (mItemClicked)
            mItemClicked = false;
    }
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
            for (int f = 0; f < OUTFIT_ITEM_COUNT; f++)
            {
                if (inventory->getItem(i)->getId() == mItems[outfit][f])
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                Item *item = inventory->getItem(i);

                if (item)
                    item->doEvent(Event::DoUnequip);
            }
        }
    }
}
