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

#include "gui/widgets/itemshortcutcontainer.h"

#include "graphics.h"
#include "inventory.h"
#include "item.h"
#include "itemshortcut.h"
#include "keyboardconfig.h"
#include "playerinfo.h"

#include "gui/inventorywindow.h"
#include "gui/gui.h"
#include "gui/itempopup.h"
#include "gui/viewport.h"

#include "resources/image.h"
#include "resources/theme.h"

#include "utils/stringutils.h"

ItemShortcutContainer::ItemShortcutContainer()
    : mItemPopup(new ItemPopup)
{
    mMaxItems = itemShortcut->getItemCount();
}

ItemShortcutContainer::~ItemShortcutContainer() = default;

void ItemShortcutContainer::draw(gcn::Graphics *graphics)
{
    cleanupFallbackItems();

    auto *g = static_cast<Graphics*>(graphics);
    auto theme = gui->getTheme();
    auto &skin = theme->getSkin(SkinType::ShortcutBox);
    const auto *drag = gui->getActiveDrag();

    graphics->setFont(getFont());

    const auto &textColor = Theme::getThemeColor(Theme::TEXT);
    const auto &equippedColor = Theme::getThemeColor(Theme::ITEM_EQUIPPED);

    for (int i = 0; i < mMaxItems; i++)
    {
        WidgetState state;
        state.x = (i % mGridWidth) * mBoxWidth;
        state.y = (i / mGridWidth) * mBoxHeight;
        skin.draw(g, state);

        // Draw item keyboard shortcut.
        const char *key = SDL_GetKeyName(
                    keyboard.getKeyValue(KeyboardConfig::KEY_SHORTCUT_1 + i));
        graphics->setColor(textColor);
        g->drawText(key,
                    state.x + skin.padding + 2,
                    state.y + skin.padding + 2,
                    gcn::Graphics::LEFT);

        const int itemId = itemShortcut->getItem(i);
        if (itemId < 0)
            continue;

        Item *item = getDisplayItem(itemId);
        const bool isGhost = item->getQuantity() == 0;
        const bool isDragged =
                drag &&
                drag->source == this &&
                drag->sourceIndex == i;
        const float alpha = isGhost ? 0.25f : (isDragged ? 0.5f : 1.0f);

        if (Image *image = item->getImage())
        {
            image->setAlpha(alpha);
            g->drawImage(image,
                        state.x + skin.padding,
                        state.y + skin.padding);
        }

        std::string caption;
        if (item->getQuantity() > 1)
            caption = toString(item->getQuantity());
        else if (item->isEquipped())
            caption = "Eq.";
        else
            continue;

        auto color = item->isEquipped() ? equippedColor : textColor;
        color.a = alpha * 255.f;

        g->setColor(color);
        g->drawText(caption,
                    state.x + mBoxWidth / 2,
                    state.y + mBoxHeight - 14,
                    gcn::Graphics::CENTER);
    }
}

Item *ItemShortcutContainer::getDisplayItem(int itemId)
{
    if (Item *item = PlayerInfo::getInventory()->findItem(itemId))
        return item;

    auto i = mFallbackItems.find(itemId);
    if (i == mFallbackItems.end())
    {
        i = mFallbackItems.emplace(
                itemId, std::make_unique<Item>(itemId)).first;
    }

    return i->second.get();
}

void ItemShortcutContainer::cleanupFallbackItems()
{
    auto *inventory = PlayerInfo::getInventory();
    for (auto i = mFallbackItems.begin(); i != mFallbackItems.end();)
    {
        const int itemId = i->first;

        if (inventory->findItem(itemId))
        {
            i = mFallbackItems.erase(i);
            continue;
        }

        bool stillReferenced = false;
        for (int slot = 0; slot < mMaxItems; ++slot)
        {
            if (itemShortcut->getItem(slot) == itemId)
            {
                stillReferenced = true;
                break;
            }
        }

        if (!stillReferenced)
            i = mFallbackItems.erase(i);
        else
            ++i;
    }
}

void ItemShortcutContainer::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (!gui->getActiveDrag() && mClickedIndex != -1)
        {
            const int itemId = itemShortcut->getItem(mClickedIndex);
            if (itemId < 0)
                return;

            Item *item = getDisplayItem(itemId);
            mItemPopup->setVisible(false);
            gui->startDrag(Drag::fromItemShortcut(item, this, mClickedIndex));
        }
    }
}

void ItemShortcutContainer::mousePressed(gcn::MouseEvent &event)
{
    mClickedIndex = getIndexFromGrid(event.getX(), event.getY());
    if (mClickedIndex == -1)
        return;

    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        // Stores the selected item if theirs one.
        if (itemShortcut->isItemSelected() && inventoryWindow->isVisible())
        {
            itemShortcut->setItem(mClickedIndex);
            itemShortcut->setItemSelected(-1);
            mClickedIndex = -1;
        }
    }
    else if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Item *item = PlayerInfo::getInventory()->
                     findItem(itemShortcut->getItem(mClickedIndex));
        if (!item)
            return;

        viewport->showPopup(nullptr, viewport->getMouseX(), viewport->getMouseY(), item);
    }
}

void ItemShortcutContainer::mouseReleased(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (itemShortcut->isItemSelected())
            itemShortcut->setItemSelected(-1);

        const int index = getIndexFromGrid(event.getX(), event.getY());
        if (index != -1 && mClickedIndex == index)
            itemShortcut->useItem(index);

        mClickedIndex = -1;
    }
}

void ItemShortcutContainer::dragFinished(const Drag &drag, DragResult result)
{
    if (result == DragResult::Ignored &&
        drag.source == this &&
        drag.sourceIndex >= 0 &&
        drag.sourceIndex < mMaxItems)
    {
        itemShortcut->removeItem(drag.sourceIndex);
    }

    mClickedIndex = -1;
}

/**
 * Handles dropping an item onto this shortcut container.
 *
 * If the drag originated from another shortcut slot, swaps or clears the
 * original slot accordingly.
 */
bool ItemShortcutContainer::handleDrop(const Drag &drag, int absX, int absY)
{
    if (!drag.item)
        return false;

    int widgetX = 0;
    int widgetY = 0;
    getAbsolutePosition(widgetX, widgetY);

    const int index = getIndexFromGrid(absX - widgetX, absY - widgetY);
    if (index == -1)
        return false;

    const int itemId = drag.item->getId();
    const int replacedItem = itemShortcut->getItem(index);
    itemShortcut->setItem(index, itemId);

    if (drag.source == this)
    {
        const int sourceIndex = drag.sourceIndex;
        if (sourceIndex >= 0 && sourceIndex != index)
            itemShortcut->setItem(sourceIndex, replacedItem);
    }

    return true;
}

// Show ItemTooltip
void ItemShortcutContainer::mouseMoved(gcn::MouseEvent &event)
{
    if (Item *item = getItemAt(event.getX(), event.getY()))
    {
        mItemPopup->setItem(item->getInfo());
        mItemPopup->position(viewport->getMouseX(), viewport->getMouseY());
    }
    else
    {
        mItemPopup->setVisible(false);
    }
}

Item *ItemShortcutContainer::getItemAt(int x, int y) const
{
    const int index = getIndexFromGrid(x, y);
    if (index == -1)
        return nullptr;

    const int itemId = itemShortcut->getItem(index);
    if (itemId < 0)
        return nullptr;

    return PlayerInfo::getInventory()->findItem(itemId);
}

// Hide ItemTooltip
void ItemShortcutContainer::mouseExited(gcn::MouseEvent &event)
{
    mItemPopup->setVisible(false);
}
