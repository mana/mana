/*
 *  The Mana World
 *  Copyright (C) 2007  The Mana World Development Team
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

#include "outfitwindow.h"

#include "configuration.h"
#include "localplayer.h"
#include "graphics.h"
#include "inventory.h"
#include "equipment.h"
#include "item.h"

#include "gui/widgets/button.h"
#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"

#include "net/inventoryhandler.h"
#include "net/net.h"

#include "resources/image.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <vector>

OutfitWindow::OutfitWindow():
    Window(_("Outfits")),
    mBoxWidth(33),
    mBoxHeight(33),
    mGridWidth(3),
    mGridHeight(3),
    mItemClicked(false),
    mItemMoved(NULL),
    mItemSelected(-1),
    mCurrentOutfit(0)
{
    setWindowName("Outfits");
    setCloseButton(true);
    setDefaultSize(250, 250, 118, 180); //160

    mPreviousButton = new Button("<", "previous", this);
    mNextButton = new Button(">", "next", this);
    mCurrentLabel = new Label("Outfit: 1");
    mCurrentLabel->setAlignment(gcn::Graphics::CENTER);
    mUnequipCheck = new CheckBox(_("Unequip first"),
                                 config.getValue("OutfitUnequip", true));

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
    for (int o = 0; o < 10; o++)
    {
        std::string outfit = config.getValue("Outfit" + toString(o), "-1");
        std::string buf;
        std::stringstream ss(outfit);

        std::vector<int> tokens;

        while (ss >> buf) {
            tokens.push_back(atoi(buf.c_str()));
        }

        for (int i = 0; i < (int)tokens.size(); i++)
        {
            mItems[o][i] = tokens[i];
        }
    }
}

void OutfitWindow::save()
{
    std::string outfitStr;
    for (int o = 0; o < 10; o++)
    {
        for (int i = 0; i < 9; i++)
        {
            outfitStr += mItems[o][i] ? toString(mItems[o][i]) : toString(-1);
            if (i <8) outfitStr += " ";
        }
        config.setValue("Outfit" + toString(o), outfitStr);
        outfitStr = "";
    }
    config.setValue("OutfitUnequip", mUnequipCheck ? true : false);
}

void OutfitWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "next")
    {
        if (mCurrentOutfit < 9) {
            mCurrentOutfit++;
        } else {
            mCurrentOutfit = 0;
        }
    }
    else if (event.getId() == "previous")
    {
        if (mCurrentOutfit > 0) {
            mCurrentOutfit--;
        } else {
            mCurrentOutfit = 9;
        }
    }
    mCurrentLabel->setCaption("Outfit: " + toString(mCurrentOutfit + 1));
}

void OutfitWindow::wearOutfit(int outfit)
{
    Item *item;
    if (mUnequipCheck->isSelected())
    {
        for (int i = 0; i < 11; i++)
        {
            //non vis is 3,4,7
            if (i != 3 && i != 4 && i != 7)
            {
                if (!(item = player_node->getInventory()->getItem(player_node
                                        ->mEquipment.get()->getEquipment(i))))
                    continue;
                Net::getInventoryHandler()->unequipItem(item);
            }
        }
    }

    for (int i = 0; i < 9; i++)
    {
        item = player_node->getInventory()->findItem(mItems[outfit][i]);
        if (item && item->getQuantity())
        {
            if (item->isEquipment()) {
                Net::getInventoryHandler()->equipItem(item);
            }
        }
    }
}

void OutfitWindow::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);
    Graphics *g = static_cast<Graphics*>(graphics);

    for (int i = 0; i < 9; i++)
    {
        const int itemX = 10 + (i % mGridWidth) * mBoxWidth;
        const int itemY = 25 + (i / mGridWidth) * mBoxHeight;

        graphics->setColor(gcn::Color(0, 0, 0, 64));
        graphics->drawRectangle(gcn::Rectangle(itemX, itemY, 32, 32));
        graphics->setColor(gcn::Color(255, 255, 255, 32));
        graphics->fillRectangle(gcn::Rectangle(itemX, itemY, 32, 32));

        if (mItems[mCurrentOutfit][i] < 0)
            continue;

        Item *item =
            player_node->getInventory()->findItem(mItems[mCurrentOutfit][i]);
        if (item) {
            // Draw item icon.
            Image* image = item->getImage();
            if (image) {
                g->drawImage(image, itemX, itemY);
            }
        }
    }
    if (mItemMoved)
    {
        // Draw the item image being dragged by the cursor.
        Image* image = mItemMoved->getImage();
        if (image)
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
    if (event.getButton() == gcn::MouseEvent::LEFT) {
        if (!mItemMoved && mItemClicked) {
            const int index = getIndexFromGrid(event.getX(), event.getY());
            if (index == -1) {
                return;
            }
            const int itemId = mItems[mCurrentOutfit][index];
            if (itemId < 0)
                return;
            Item *item = player_node->getInventory()->findItem(itemId);
            if (item)
            {
                mItemMoved = item;
                mItems[mCurrentOutfit][index] = -1;
            }
        }
        if (mItemMoved) {
            mCursorPosX = event.getX();
            mCursorPosY = event.getY();
        }
    }
}

void OutfitWindow::mousePressed(gcn::MouseEvent &event)
{
    Window::mousePressed(event);
    const int index = getIndexFromGrid(event.getX(), event.getY());
    if (index == -1) {
        return;
    }

    // Stores the selected item if there is one.
    if (isItemSelected()) {
        mItems[mCurrentOutfit][index] = mItemSelected;
        mItemSelected = -1;
    }
    else if (mItems[mCurrentOutfit][index]) {
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
        if (index == -1) {
            mItemMoved = NULL;
            return;
        }
        if (mItemMoved) {
            mItems[mCurrentOutfit][index] = mItemMoved->getId();
            mItemMoved = NULL;
        }
        if (mItemClicked) {
            mItemClicked = false;
        }
    }
}

int OutfitWindow::getIndexFromGrid(int pointX, int pointY) const
{
    const gcn::Rectangle tRect = gcn::Rectangle(
        10, 25, 10 + mGridWidth * mBoxWidth, 25 + mGridHeight * mBoxHeight);
    if (!tRect.isPointInRect(pointX, pointY)) {
        return -1;
    }
    const int index = (((pointY - 25) / mBoxHeight) * mGridWidth) +
        (pointX - 10) / mBoxWidth;
    if (index >= 9)
    {
        return -1;
    }
    return index;
}
