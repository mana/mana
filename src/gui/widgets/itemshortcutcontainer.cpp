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

#include "gui/widgets/itemshortcutcontainer.h"

#include "graphics.h"
#include "inventory.h"
#include "item.h"
#include "itemshortcut.h"
#include "keyboardconfig.h"
#include "playerinfo.h"

#include "gui/inventorywindow.h"
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
    auto *g = static_cast<Graphics*>(graphics);
    auto theme = gui->getTheme();
    auto &skin = theme->getSkin(SkinType::ShortcutBox);

    graphics->setFont(getFont());

    for (int i = 0; i < mMaxItems; i++)
    {
        WidgetState state;
        state.x = (i % mGridWidth) * mBoxWidth;
        state.y = (i / mGridWidth) * mBoxHeight;
        skin.draw(g, state);

        // Draw item keyboard shortcut.
        const char *key = SDL_GetKeyName(
                    keyboard.getKeyValue(KeyboardConfig::KEY_SHORTCUT_1 + i));
        graphics->setColor(Theme::getThemeColor(Theme::TEXT));
        g->drawText(key,
                    state.x + skin.padding + 2,
                    state.y + skin.padding + 2,
                    gcn::Graphics::LEFT);

        const int itemId = itemShortcut->getItem(i);
        if (itemId < 0)
            continue;

        if (Item *item = PlayerInfo::getInventory()->findItem(itemId))
        {
            // Draw item icon.
            if (Image *image = item->getImage())
            {
                std::string caption;
                if (item->getQuantity() > 1)
                    caption = toString(item->getQuantity());
                else if (item->isEquipped())
                    caption = "Eq.";

                image->setAlpha(1.0f);
                g->drawImage(image, state.x + skin.padding, state.y + skin.padding);
                if (item->isEquipped())
                    g->setColor(Theme::getThemeColor(Theme::ITEM_EQUIPPED));
                g->drawText(caption,
                            state.x + mBoxWidth / 2,
                            state.y + mBoxHeight - 14,
                            gcn::Graphics::CENTER);
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
            g->drawText(toString(mItemMoved->getQuantity()),
                        tPosX + mBoxWidth / 2, tPosY + mBoxHeight - 14,
                        gcn::Graphics::CENTER);
        }
    }
}

void ItemShortcutContainer::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (!mItemMoved && mItemClicked)
        {
            const int index = getIndexFromGrid(event.getX(), event.getY());
            if (index == -1)
                return;

            const int itemId = itemShortcut->getItem(index);
            if (itemId < 0)
                return;

            if (Item *item = PlayerInfo::getInventory()->findItem(itemId))
            {
                mItemMoved = item;
                itemShortcut->removeItem(index);
            }
        }

        if (mItemMoved)
        {
            mCursorPosX = event.getX();
            mCursorPosY = event.getY();
        }
    }
}

void ItemShortcutContainer::mousePressed(gcn::MouseEvent &event)
{
    const int index = getIndexFromGrid(event.getX(), event.getY());

    if (index == -1)
        return;

    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        // Stores the selected item if theirs one.
        if (itemShortcut->isItemSelected() && inventoryWindow->isVisible())
        {
            itemShortcut->setItem(index);
            itemShortcut->setItemSelected(-1);
        }
        else if (itemShortcut->getItem(index))
            mItemClicked = true;
    }
    else if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Item *item = PlayerInfo::getInventory()->
                     findItem(itemShortcut->getItem(index));

        if (!item)
            return;

        // Convert relative to the window coordinates to absolute screen
        // coordinates.
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
        if (index == -1)
        {
            mItemMoved = nullptr;
            return;
        }
        if (mItemMoved)
        {
            itemShortcut->setItems(index, mItemMoved->getId());
            mItemMoved = nullptr;
        }
        else if (itemShortcut->getItem(index) && mItemClicked)
        {
            itemShortcut->useItem(index);
        }

        if (mItemClicked)
            mItemClicked = false;
    }
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
