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

#include "gui/itemcontainer.h"

#include "gui/chat.h"
#include "gui/itempopup.h"
#include "gui/palette.h"
#include "gui/sdlinput.h"
#include "gui/viewport.h"

#include "net/net.h"
#include "net/inventoryhandler.h"

#include "graphics.h"
#include "inventory.h"
#include "item.h"
#include "itemshortcut.h"
#include "log.h"

#include "resources/image.h"
#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"

#include "utils/stringutils.h"

#include <guichan/mouseinput.hpp>
#include <guichan/selectionlistener.hpp>

// TODO: Add support for adding items to the item shortcut window (global
// itemShortcut).

static const int BOX_WIDTH = 35;
static const int BOX_HEIGHT = 43;

ItemContainer::ItemContainer(Inventory *inventory, bool forceQuantity):
    mInventory(inventory),
    mGridColumns(1),
    mGridRows(1),
    mSelectedIndex(-1),
    mHighlightedIndex(-1),
    mLastUsedSlot(-1),
    mSelectionStatus(SEL_NONE),
    mForceQuantity(forceQuantity),
    mSwapItems(false),
    mDescItems(false)
{
    mItemPopup = new ItemPopup;
    setFocusable(true);

    ResourceManager *resman = ResourceManager::getInstance();

    mSelImg = resman->getImage("graphics/gui/selection.png");
    if (!mSelImg)
        logger->error("Unable to load selection.png");

    addKeyListener(this);
    addMouseListener(this);
    addWidgetListener(this);
}

ItemContainer::~ItemContainer()
{
    mSelImg->decRef();
    delete mItemPopup;
}

void ItemContainer::logic()
{
    gcn::Widget::logic();

    const int lastUsedSlot = mInventory->getLastUsedSlot();

    if (lastUsedSlot != mLastUsedSlot)
    {
        mLastUsedSlot = lastUsedSlot;
        adjustHeight();
    }
}

void ItemContainer::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    g->setFont(getFont());

    for (int i = 0; i < mGridColumns; i++)
    {
        for (int j = 0; j < mGridRows; j++)
        {
            int itemX = i * BOX_WIDTH;
            int itemY = j * BOX_HEIGHT;
            int itemIndex = (j * mGridColumns) + i;
            Item *item = mInventory->getItem(itemIndex);

            if (!item || item->getId() == 0)
                continue;

            Image *image = item->getImage();
            if (image)
            {
                if (itemIndex == mSelectedIndex)
                {
                    if (mSelectionStatus == SEL_DRAGGING) {
                        // Reposition the coords to that of the cursor.
                        itemX = mDragPosX - (BOX_WIDTH / 2);
                        itemY = mDragPosY - (BOX_HEIGHT / 2);
                    }
                    else {
                        // Draw selected image.
                        g->drawImage(mSelImg, itemX, itemY);
                    }
                }
                g->drawImage(image, itemX, itemY);
            }
            // Draw item caption
            std::string caption;
            if (item->getQuantity() > 1 || mForceQuantity)
                caption = toString(item->getQuantity());
            else if (item->isEquipped())
                caption = "Eq.";

            if (item->isEquipped())
                g->setColor(guiPalette->getColor(Palette::ITEM_EQUIPPED));
            else
                g->setColor(gcn::Color(0, 0, 0));

            g->drawText(caption, itemX + BOX_WIDTH / 2,
                        itemY + BOX_HEIGHT - 14, gcn::Graphics::CENTER);
        }
    }

    // Draw an orange box around the selected item
    if (isFocused() && mHighlightedIndex != -1)
    {
        const int itemX = (mHighlightedIndex % mGridColumns) * BOX_WIDTH;
        const int itemY = (mHighlightedIndex / mGridColumns) * BOX_HEIGHT;
        g->setColor(gcn::Color(255, 128, 0));
        g->drawRectangle(gcn::Rectangle(itemX, itemY, BOX_WIDTH, BOX_HEIGHT));
    }
}

void ItemContainer::selectNone()
{
    setSelectedIndex(-1);
    mSelectionStatus = SEL_NONE;
}

void ItemContainer::setSelectedIndex(int newIndex)
{
    if (mSelectedIndex != newIndex)
    {
        mSelectedIndex = newIndex;
        distributeValueChangedEvent();
    }
}

Item *ItemContainer::getSelectedItem() const
{
    return mInventory->getItem(mSelectedIndex);
}

void ItemContainer::distributeValueChangedEvent()
{
    SelectionListenerIterator i, i_end;

    for (i = mSelectionListeners.begin(), i_end = mSelectionListeners.end();
         i != i_end; ++i)
    {
        gcn::SelectionEvent event(this);
        (*i)->valueChanged(event);
    }
}

void ItemContainer::keyPressed(gcn::KeyEvent &event)
{
    /*switch (event.getKey().getValue())
    {
        case Key::LEFT:
            moveHighlight(Left);
            break;
        case Key::RIGHT:
            moveHighlight(Right);
            break;
        case Key::UP:
            moveHighlight(Up);
            break;
        case Key::DOWN:
            moveHighlight(Down);
            break;
        case Key::SPACE:
            keyAction();
            break;
        case Key::LEFT_ALT:
        case Key::RIGHT_ALT:
            mSwapItems = true;
            break;
        case Key::RIGHT_CONTROL:
            mDescItems = true;
            break;
    }*/
}

void ItemContainer::keyReleased(gcn::KeyEvent &event)
{
    /*switch (event.getKey().getValue())
    {
        case Key::LEFT_ALT:
        case Key::RIGHT_ALT:
            mSwapItems = false;
            break;
        case Key::RIGHT_CONTROL:
            mDescItems = false;
            break;
    }*/
}

void ItemContainer::mousePressed(gcn::MouseEvent &event)
{
    const int button = event.getButton();
    if (button == gcn::MouseEvent::LEFT || button == gcn::MouseEvent::RIGHT)
    {
        const int index = getSlotIndex(event.getX(), event.getY());
        if (index == Inventory::NO_SLOT_INDEX)
            return;

        Item *item = mInventory->getItem(index);

        // put item name into chat window
        if (mDescItems)
        {
            chatWindow->addItemText(item->getInfo().getName());
        }

        if (mSelectedIndex == index)
        {
            mSelectionStatus = SEL_DESELECTING;
        }
        else if (item && item->getId())
        {
            setSelectedIndex(index);
            mSelectionStatus = SEL_SELECTING;

            itemShortcut->setItemSelected(item->getId());
        }
        else
        {
            selectNone();
        }
    }
}

void ItemContainer::mouseDragged(gcn::MouseEvent &event)
{
    if (mSelectionStatus != SEL_NONE)
    {
        mSelectionStatus = SEL_DRAGGING;
        mDragPosX = event.getX();
        mDragPosY = event.getY();
    }
}

void ItemContainer::mouseReleased(gcn::MouseEvent &event)
{
    switch (mSelectionStatus)
    {
        case SEL_SELECTING:
            mSelectionStatus = SEL_SELECTED;
            return;
        case SEL_DESELECTING:
            selectNone();
            return;
        case SEL_DRAGGING:
            mSelectionStatus = SEL_SELECTED;
            break;
        default:
            return;
    };

    int index = getSlotIndex(event.getX(), event.getY());
    if (index == Inventory::NO_SLOT_INDEX)
        return;
    if (index == mSelectedIndex || mSelectedIndex == -1)
        return;
    Net::getInventoryHandler()->moveItem(mSelectedIndex, index);
    selectNone();
}


// Show ItemTooltip
void ItemContainer::mouseMoved(gcn::MouseEvent &event)
{
    Item *item = mInventory->getItem(getSlotIndex(event.getX(), event.getY()));

    if (item)
    {
        mItemPopup->setItem(item->getInfo());
        mItemPopup->view(viewport->getMouseX(), viewport->getMouseY());
    }
    else
    {
        mItemPopup->setVisible(false);
    }
}

// Hide ItemTooltip
void ItemContainer::mouseExited(gcn::MouseEvent &event)
{
    mItemPopup->setVisible(false);
}

void ItemContainer::widgetResized(const gcn::Event &event)
{
    mGridColumns = std::max(1, getWidth() / BOX_WIDTH);
    adjustHeight();
}

void ItemContainer::adjustHeight()
{
    mGridRows = (mLastUsedSlot + 1) / mGridColumns;
    if (mGridRows == 0 || (mLastUsedSlot + 1) % mGridColumns > 0)
        ++mGridRows;

    setHeight(mGridRows * BOX_HEIGHT);
}

int ItemContainer::getSlotIndex(int x, int y) const
{
    if (x < getWidth() && y < getHeight())
    {
        return (y / BOX_HEIGHT) * mGridColumns + (x / BOX_WIDTH);
    }
    return Inventory::NO_SLOT_INDEX;
}

void ItemContainer::keyAction()
{
    // If there is no highlight then return.
    if (mHighlightedIndex == -1)
        return;

    // If the highlight is on the selected item, then deselect it.
    if (mHighlightedIndex == mSelectedIndex)
    {
         selectNone();
    }
    // Check and swap items if necessary.
    else if (mSwapItems &&
        mSelectedIndex != -1 &&
        mHighlightedIndex != -1)
    {
        Net::getInventoryHandler()->moveItem(
            mSelectedIndex, mHighlightedIndex);
        setSelectedIndex(mHighlightedIndex);
    }
    // If the highlight is on an item then select it.
    else if (mHighlightedIndex != -1)
    {
        setSelectedIndex(mHighlightedIndex);
        mSelectionStatus = SEL_SELECTED;
    }
    // If the highlight is on a blank space then move it.
    else if (mSelectedIndex != -1)
    {
        Net::getInventoryHandler()->moveItem(
            mSelectedIndex, mHighlightedIndex);
        selectNone();
    }
}

void ItemContainer::moveHighlight(Direction direction)
{
    if (mHighlightedIndex == -1)
    {
        if (mSelectedIndex != -1) {
            mHighlightedIndex = mSelectedIndex;
        }
        else {
            mHighlightedIndex = 0;
        }
        return;
    }

    switch (direction)
    {
        case Left:
            if (mHighlightedIndex % mGridColumns == 0)
            {
                mHighlightedIndex += mGridColumns;
            }
            mHighlightedIndex--;
            break;
        case Right:
            if ((mHighlightedIndex % mGridColumns) ==
                (mGridColumns - 1))
            {
                mHighlightedIndex -= mGridColumns;
            }
            mHighlightedIndex++;
            break;
        case Up:
            if (mHighlightedIndex / mGridColumns == 0)
            {
                mHighlightedIndex += (mGridColumns * mGridRows);
            }
            mHighlightedIndex -= mGridColumns;
            break;
        case Down:
            if ((mHighlightedIndex / mGridColumns) ==
                (mGridRows - 1))
            {
                mHighlightedIndex -= (mGridColumns * mGridRows);
            }
            mHighlightedIndex += mGridColumns;
            break;
    }
}
