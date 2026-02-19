/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "gui/widgets/itemcontainer.h"

#include "graphics.h"
#include "inventory.h"
#include "item.h"
#include "itemshortcut.h"

#include "gui/chatwindow.h"
#include "gui/gui.h"
#include "gui/itemamountwindow.h"
#include "gui/itempopup.h"
#include "gui/outfitwindow.h"
#include "gui/tradewindow.h"
#include "gui/viewport.h"

#include "resources/image.h"
#include "resources/iteminfo.h"
#include "resources/theme.h"

#include "utils/stringutils.h"

#include <guichan/mouseinput.hpp>
#include <guichan/selectionlistener.hpp>

ItemContainer::ItemContainer(Inventory *inventory):
    mInventory(inventory)
{
    mItemPopup = std::make_unique<ItemPopup>();
    mItemPopup->addDeathListener(this);

    setFocusable(true);

    addKeyListener(this);
    addMouseListener(this);
    addWidgetListener(this);
}

ItemContainer::~ItemContainer() = default;

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
    auto *g = static_cast<Graphics*>(graphics);
    const auto *drag = gui->getActiveDrag();

    g->setFont(getFont());

    mFilteredMap.clear();
    int currentIndex = 0;
    //Filter checking
    for (int i = 0; i < mGridColumns; i++)
    {
        for (int j = 0; j < mGridRows; j++)
        {
            int itemIndex = j * mGridColumns + i;
            Item *item = mInventory->getItem(itemIndex);
            if (!item || item->getId() == 0)
                continue;

            if (!mFilter.empty())
            {
                if (normalize(item->getInfo().name).find(mFilter) == std::string::npos)
                    continue;
                mFilteredMap[currentIndex] = item;
                currentIndex++;
            }
            else
            {
                mFilteredMap[itemIndex] = item;
            }
        }
    }

    auto theme = gui->getTheme();
    auto &slotSkin = theme->getSkin(SkinType::ItemSlot);
    WidgetState slotState;

    for (int i = 0; i < mGridColumns; i++)
    {
        for (int j = 0; j < mGridRows; j++)
        {
            int itemX = i * slotSkin.width;
            int itemY = j * slotSkin.height;
            int itemIndex = j * mGridColumns + i;

            slotState.x = itemX;
            slotState.y = itemY;
            slotState.flags = 0;

            if (itemIndex == mSelectedIndex)
            {
                slotState.flags |= STATE_SELECTED;
            }

            slotSkin.draw(g, slotState);

            Item *item = getItemAt(itemIndex);

            if (!item || item->getId() == 0)
                continue;

            if (Image *image = item->getImage())
            {
                const bool isDragSourceItem =
                        drag &&
                        drag->source == this &&
                        drag->item &&
                        drag->item->getInvIndex() == item->getInvIndex();

                image->setAlpha(isDragSourceItem ? 0.5f : 1.0f);
                g->drawImage(image, itemX + slotSkin.padding, itemY + slotSkin.padding);
            }

            // Draw item caption
            std::string caption;
            if (item->getQuantity() > 1)
                caption = toString(item->getQuantity());
            else if (item->isEquipped())
                caption = "Eq.";

            if (item->isEquipped())
                g->setColor(theme->getColor(Theme::ITEM_EQUIPPED));
            else
                g->setColor(theme->getColor(Theme::TEXT));

            g->drawText(caption, itemX + slotSkin.width / 2,
                        itemY + slotSkin.height - 14, gcn::Graphics::CENTER);
        }
    }

    // Draw an orange box around the selected item
    if (isFocused() && mHighlightedIndex != -1)
    {
        const int itemX = (mHighlightedIndex % mGridColumns) * slotSkin.width;
        const int itemY = (mHighlightedIndex / mGridColumns) * slotSkin.height;
        g->setColor(gcn::Color(255, 128, 0));
        g->drawRectangle(gcn::Rectangle(itemX, itemY, slotSkin.width, slotSkin.height));
    }
}

void ItemContainer::selectNone()
{
    setSelectedIndex(-1);
    mSelectionStatus = SEL_NONE;
    itemShortcut->setItemSelected(-1);
    outfitWindow->setItemSelected(-1);
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
    return getItemAt(mSelectedIndex);
}

Item *ItemContainer::getItemAt(int index) const
{
    auto i = mFilteredMap.find(index);
    return i == mFilteredMap.end() ? nullptr : i->second;
}

void ItemContainer::setFilter(const std::string &filter)
{
    mFilter = normalize(filter);
}

void ItemContainer::distributeValueChangedEvent()
{
    for (auto listener : mSelectionListeners)
    {
        gcn::SelectionEvent event(this);
        listener->valueChanged(event);
    }
}

void ItemContainer::hidePopup()
{
    mItemPopup->setVisible(false);
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
        mClickedIndex = getSlotIndex(event.getX(), event.getY());
        Item *item = getItemAt(mClickedIndex);

        if (!item)
        {
            mSelectionStatus = SEL_DESELECTING;
            return;
        }

        // put item name into chat window
        if (mDescItems)
            chatWindow->addItemText(item->getInfo().name);

        if (mSelectedIndex != mClickedIndex)
        {
            if (item->getId())
            {
                setSelectedIndex(mClickedIndex);
                mSelectionStatus = SEL_SELECTING;
                itemShortcut->setItemSelected(item->getId());

                if (item->isEquippable())
                    outfitWindow->setItemSelected(item->getId());
            }
            else
            {
                selectNone();
            }
        }
    }
}

void ItemContainer::mouseDragged(gcn::MouseEvent &event)
{
    if (!mDragEnabled || event.getButton() != gcn::MouseEvent::LEFT)
        return;

    if (mSelectionStatus != SEL_NONE &&
        mClickedIndex != NO_SLOT_INDEX)
    {
        Item *item = mInventory->getItem(mClickedIndex);
        if (!item)
            return;

        if (!gui->getActiveDrag())
        {
            Drag::SourceType sourceType;
            switch (mInventory->getType())
            {
                case Inventory::INVENTORY:
                    sourceType = Drag::SourceType::Inventory;
                    break;
                case Inventory::STORAGE:
                    sourceType = Drag::SourceType::Storage;
                    break;
                default:
                    return;
            }

            hidePopup();
            gui->startDrag(Drag(sourceType, item, this));
        }

        mSelectionStatus = SEL_DRAGGING;
    }
}

void ItemContainer::mouseReleased(gcn::MouseEvent &event)
{
    switch (mSelectionStatus)
    {
        case SEL_SELECTING:
            mSelectionStatus = SEL_SELECTED;
            break;
        case SEL_DESELECTING:
            selectNone();
            break;
        case SEL_DRAGGING:
        default:
            break;
    };
}

/**
 * Handles dragging of items between containers. This includes:
 * - Dragging items between inventory and storage
 * - Dragging items to the trade window
 */
bool ItemContainer::handleDrop(const Drag &drag, int /*absX*/, int /*absY*/)
{
    if (drag.sourceType != Drag::SourceType::Inventory &&
        drag.sourceType != Drag::SourceType::Storage)
        return false;

    Item *item = drag.item.get();
    if (!item || drag.source == this)
        return false;

    const auto sourceType =
            (drag.sourceType == Drag::SourceType::Inventory)
            ? Inventory::INVENTORY
            : Inventory::STORAGE;
    const auto targetType = mInventory->getType();

    if (sourceType == Inventory::INVENTORY &&
        targetType == Inventory::STORAGE)
    {
        Event moveEvent = item->createEvent(Event::DoMove);
        moveEvent.setInt("amount", item->getQuantity());
        moveEvent.setInt("source", Inventory::INVENTORY);
        moveEvent.setInt("destination", Inventory::STORAGE);
        moveEvent.trigger(Event::ItemChannel);
        return true;
    }

    if (sourceType == Inventory::STORAGE &&
        targetType == Inventory::INVENTORY)
    {
        Event moveEvent = item->createEvent(Event::DoMove);
        moveEvent.setInt("amount", item->getQuantity());
        moveEvent.setInt("source", Inventory::STORAGE);
        moveEvent.setInt("destination", Inventory::INVENTORY);
        moveEvent.trigger(Event::ItemChannel);
        return true;
    }

    if (sourceType == Inventory::INVENTORY &&
        targetType == Inventory::TRADE &&
        acceptsTradeDrops() &&
        tradeWindow &&
        tradeWindow->isVisible())
    {
        ItemAmountWindow::showWindow(ItemAmountWindow::TradeAdd,
                                     tradeWindow, item);
        return true;
    }

    return false;
}

void ItemContainer::dragFinished(const Drag &/*drag*/, DragResult result)
{
    mClickedIndex = NO_SLOT_INDEX;      // Avoid drag resuming on mouseDragged
    mSelectionStatus = SEL_SELECTED;
    if (result == DragResult::Accepted)
        selectNone();
}

// Show ItemTooltip
void ItemContainer::mouseMoved(gcn::MouseEvent &event)
{
    if (Item *item = getItemAt(getSlotIndex(event.getX(), event.getY())))
    {
        mItemPopup->setItem(item->getInfo());
        mItemPopup->position(viewport->getMouseX(), viewport->getMouseY());
    }
    else
    {
        hidePopup();
    }
}

// Hide ItemTooltip
void ItemContainer::mouseExited(gcn::MouseEvent &event)
{
    hidePopup();
}

void ItemContainer::widgetResized(const gcn::Event &event)
{
    auto &slotSkin = gui->getTheme()->getSkin(SkinType::ItemSlot);

    mGridColumns = std::max(1, getWidth() / slotSkin.width);
    adjustHeight();
}

void ItemContainer::death(const gcn::Event &event)
{
    // If somebody else killed the PopupUp, make sure we don't also try to delete it
    if (event.getSource() == mItemPopup.get())
        mItemPopup.release();
}

void ItemContainer::adjustHeight()
{
    auto &slotSkin = gui->getTheme()->getSkin(SkinType::ItemSlot);

    mGridRows = (mLastUsedSlot + 1) / mGridColumns;
    if (mGridRows == 0 || (mLastUsedSlot + 1) % mGridColumns > 0)
        ++mGridRows;

    setHeight(mGridRows * slotSkin.height);
}

int ItemContainer::getSlotIndex(int x, int y) const
{
    if (x >= getWidth() || y >= getHeight())
        return NO_SLOT_INDEX;

    auto &slotSkin = gui->getTheme()->getSkin(SkinType::ItemSlot);
    const auto row = y / slotSkin.height;
    const auto column = x / slotSkin.width;

    if (row < 0 || row >= mGridRows || column < 0 || column >= mGridColumns)
        return NO_SLOT_INDEX;

    return (row * mGridColumns) + column;
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
        Item *item = getSelectedItem();
        Event event = item->createEvent(Event::DoMove);
        event.setInt("newIndex", mHighlightedIndex);
        event.trigger(Event::ItemChannel);
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
        Item *item = getSelectedItem();
        Event event = item->createEvent(Event::DoMove);
        event.setInt("newIndex", mHighlightedIndex);
        event.trigger(Event::ItemChannel);
        selectNone();
    }
}

void ItemContainer::moveHighlight(Direction direction)
{
    if (mHighlightedIndex == -1)
    {
        if (mSelectedIndex != -1)
            mHighlightedIndex = mSelectedIndex;
        else
            mHighlightedIndex = 0;
        return;
    }

    switch (direction)
    {
        case Left:
            if (mHighlightedIndex % mGridColumns == 0)
                mHighlightedIndex += mGridColumns;
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
                mHighlightedIndex += (mGridColumns * mGridRows);
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
