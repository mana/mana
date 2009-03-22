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

#include "itemcontainer.h"
#include "chat.h"

#include "itempopup.h"

#include <guichan/mouseinput.hpp>
#include <guichan/selectionlistener.hpp>

#include "sdlinput.h"

#include "../graphics.h"
#include "../inventory.h"
#include "../item.h"
#include "../itemshortcut.h"
#include "../localplayer.h"
#include "../log.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

#include "../utils/stringutils.h"

// TODO: Add support for adding items to the item shortcut window (global
// itemShortcut).

static const int BOX_WIDTH = 36;
static const int BOX_HEIGHT = 44;

enum
{
    SEL_NONE = 0,
    SEL_SELECTED,
    SEL_SELECTING,
    SEL_DESELECTING,
    SEL_DRAGGING
};

ItemContainer::ItemContainer(Inventory *inventory,
                             int gridColumns,
                             int gridRows,
                             int offset):
    mInventory(inventory),
    mGridColumns(gridColumns),
    mGridRows(gridRows),
    mOffset(offset),
    mSelectedItem(NULL),
    mHighlightedItem(NULL),
    mSelectionStatus(SEL_NONE),
    mSwapItems(false),
    mDescItems(false)
{
    mItemPopup = new ItemPopup();
    setFocusable(true);

    ResourceManager *resman = ResourceManager::getInstance();

    mSelImg = resman->getImage("graphics/gui/selection.png");
    if (!mSelImg) logger->error("Unable to load selection.png");

    addKeyListener(this);
    addMouseListener(this);

    setSize((BOX_WIDTH - 1) * mGridColumns + 1,
            (BOX_HEIGHT - 1) * mGridRows + 1);
}

ItemContainer::~ItemContainer()
{
    mSelImg->decRef();
    delete mItemPopup;
}

void ItemContainer::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    for (int i = 0; i < mGridColumns; i++)
    {
        for (int j = 0; j < mGridRows; j++)
        {
            // Items positions made to overlap on another.
            int itemX = i * (BOX_WIDTH - 1);
            int itemY = j * (BOX_HEIGHT - 1);

            // Set color to black.
            g->setColor(gcn::Color(0, 0, 0));
            // Draw box border.
            g->drawRectangle(
                gcn::Rectangle(itemX, itemY, BOX_WIDTH, BOX_HEIGHT));

            Item *item = mInventory->getItem((j * mGridColumns) + i);

            if (!item || item->getId() == 0)
                continue;

            Image *image = item->getImage();
            if (image)
            {
                if (item == mSelectedItem)
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
            if (item->getQuantity() > 1) {
                // Draw item caption
                g->drawText(
                    toString(item->getQuantity()),
                    itemX + BOX_WIDTH / 2,
                    itemY + BOX_HEIGHT - 14,
                    gcn::Graphics::CENTER);
            }

        }
    }

    if (isFocused() && mHighlightedItem) {
        // Items positions made to overlap on another.
        const int i = mHighlightedItem->getInvIndex();
        const int itemX = (i % mGridColumns) * (BOX_WIDTH - 1);
        const int itemY = (i / mGridColumns) * (BOX_HEIGHT - 1);
        // Set color to orange.
        g->setColor(gcn::Color(255, 128, 0));
        // Draw box border.
        g->drawRectangle(gcn::Rectangle(itemX, itemY, BOX_WIDTH, BOX_HEIGHT));
    }
}

void ItemContainer::selectNone()
{
    setSelectedItem(NULL);
}

void ItemContainer::setSelectedItem(Item *item)
{
    if (mSelectedItem != item)
    {
        mSelectedItem = item;
        distributeValueChangedEvent();
    }
}

void ItemContainer::distributeValueChangedEvent()
{
    SelectionListenerIterator iter;

    for (iter = mSelectionListeners.begin(); iter != mSelectionListeners.end();
         ++iter)
    {
        gcn::SelectionEvent event(this);
        (*iter)->valueChanged(event);
    }
}

void ItemContainer::keyPressed(gcn::KeyEvent &event)
{
    switch (event.getKey().getValue())
    {
        case Key::LEFT:
            moveHighlight(MOVE_SELECTED_LEFT);
            break;
        case Key::RIGHT:
            moveHighlight(MOVE_SELECTED_RIGHT);
            break;
        case Key::UP:
            moveHighlight(MOVE_SELECTED_UP);
            break;
        case Key::DOWN:
            moveHighlight(MOVE_SELECTED_DOWN);
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
    }
}

void ItemContainer::keyReleased(gcn::KeyEvent &event)
{
    switch (event.getKey().getValue())
    {
        case Key::LEFT_ALT:
        case Key::RIGHT_ALT:
            mSwapItems = false;
            break;
        case Key::RIGHT_CONTROL:
            mDescItems = false;
            break;
    }
}

void ItemContainer::mousePressed(gcn::MouseEvent &event)
{
    const int button = event.getButton();
    if (button == gcn::MouseEvent::LEFT || button == gcn::MouseEvent::RIGHT)
    {
        const int index = getSlotIndex(event.getX(), event.getY());
        if (index == Inventory::NO_SLOT_INDEX) {
            return;
        }

        Item *item = mInventory->getItem(index);

        // put item name into chat window
        if (mDescItems)
        {
            chatWindow->addItemText(item->getInfo().getName());
        }

        if (mSelectedItem && mSelectedItem == item)
        {
            mSelectionStatus = SEL_DESELECTING;
        }
        else if (item && item->getId())
        {
            setSelectedItem(item);
            mSelectionStatus = SEL_SELECTING;

            itemShortcut->setItemSelected(item->getId());
        }
        else
        {
            setSelectedItem(NULL);
            mSelectionStatus = SEL_NONE;
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
            setSelectedItem(NULL);
            mSelectionStatus = SEL_NONE;
            return;
        case SEL_DRAGGING:
            mSelectionStatus = SEL_SELECTED;
            break;
        default:
            return;
    };

    int index = getSlotIndex(event.getX(), event.getY());
    if (index == Inventory::NO_SLOT_INDEX) return;
    Item *item = mInventory->getItem(index);
    if (item == mSelectedItem) return;
    player_node->moveInvItem(mSelectedItem, index);
    setSelectedItem(NULL);
    mSelectionStatus = SEL_NONE;
}


// Show ItemTooltip
void ItemContainer::mouseMoved(gcn::MouseEvent &event)
{
    Item *item = mInventory->getItem(getSlotIndex(event.getX(), event.getY()));

    if (item)
    {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        mItemPopup->setItem(item->getInfo());
        mItemPopup->setOpaque(false);
        mItemPopup->view(mouseX, mouseY);
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

int ItemContainer::getSlotIndex(const int posX, const int posY) const
{
    if (getDimension().isPointInRect(posX, posY))
    {
        // Takes into account, boxes are overlapping each other.
        return (posY / (BOX_HEIGHT - 1)) * mGridColumns + (posX / (BOX_WIDTH - 1));
    }
    return Inventory::NO_SLOT_INDEX;
}

void ItemContainer::keyAction()
{
    // If there is no highlight then return.
    if (!mHighlightedItem)
        return;

    // If the highlight is on the selected item, then deselect it.
    if (mHighlightedItem == mSelectedItem)
    {
        setSelectedItem(NULL);
        mSelectionStatus = SEL_NONE;
    }
    // Check and swap items if necessary.
    else if (mSwapItems &&
        mSelectedItem &&
        mHighlightedItem->getId())
    {
        player_node->moveInvItem(
            mSelectedItem, mHighlightedItem->getInvIndex());
        setSelectedItem(mHighlightedItem);
    }
    // If the highlight is on an item then select it.
    else if (mHighlightedItem->getId())
    {
        setSelectedItem(mHighlightedItem);
        mSelectionStatus = SEL_SELECTED;
    }
    // If the highlight is on a blank space then move it.
    else if (mSelectedItem)
    {
        player_node->moveInvItem(
            mSelectedItem, mHighlightedItem->getInvIndex());
        setSelectedItem(NULL);
        mSelectionStatus = SEL_NONE;
    }
}

void ItemContainer::moveHighlight(int direction)
{
    if (!mHighlightedItem)
    {
        if (mSelectedItem) {
            mHighlightedItem = mSelectedItem;
        }
        else {
            mHighlightedItem = mInventory->getItem(0);
        }
        return;
    }

    switch (direction)
    {
        case MOVE_SELECTED_LEFT:
            if (mHighlightedItem->getInvIndex() % mGridColumns == 0)
            {
                mHighlightedItem += mGridColumns;
            }
            mHighlightedItem--;
            break;
        case MOVE_SELECTED_RIGHT:
            if ((mHighlightedItem->getInvIndex() % mGridColumns) ==
                (mGridColumns - 1))
            {
                mHighlightedItem -= mGridColumns;
            }
            mHighlightedItem++;
            break;
        case MOVE_SELECTED_UP:
            if (mHighlightedItem->getInvIndex() / mGridColumns == 0)
            {
                mHighlightedItem += (mGridColumns * mGridRows);
            }
            mHighlightedItem -= mGridColumns;
            break;
        case MOVE_SELECTED_DOWN:
            if ((mHighlightedItem->getInvIndex() / mGridColumns) ==
                (mGridRows - 1))
            {
                mHighlightedItem -= (mGridColumns * mGridRows);
            }
            mHighlightedItem += mGridColumns;
            break;
    }
}
