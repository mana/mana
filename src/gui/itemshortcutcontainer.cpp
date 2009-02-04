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
#include <SDL_mouse.h>

#include "gui.h"
#include "itemshortcutcontainer.h"
#include "itempopup.h"
#include "viewport.h"

#include "../configuration.h"
#include "../graphics.h"
#include "../inventory.h"
#include "../item.h"
#include "../itemshortcut.h"
#include "../keyboardconfig.h"
#include "../localplayer.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

#include "../utils/tostring.h"

ItemShortcutContainer::ItemShortcutContainer():
    ShortcutContainer(),
    mItemClicked(false),
    mItemMoved(NULL)
{
    addMouseListener(this);
    addWidgetListener(this);

    mItemPopup = new ItemPopup();

    ResourceManager *resman = ResourceManager::getInstance();

    mBackgroundImg = resman->getImage("graphics/gui/item_shortcut_bgr.png");
    mMaxItems = itemShortcut->getItemCount();

    mBackgroundImg->setAlpha(config.getValue("guialpha", 0.8));

    mBoxHeight = mBackgroundImg->getHeight();
    mBoxWidth = mBackgroundImg->getWidth();
}

ItemShortcutContainer::~ItemShortcutContainer()
{
    mBackgroundImg->decRef();
    delete mItemPopup;
}

void ItemShortcutContainer::logic()
{
    gcn::Widget::logic();

    int i = itemShortcut->getItemCount();

    if (i != mMaxItems)
    {
        mMaxItems = i;
        setWidth(getWidth());
    }
}

void ItemShortcutContainer::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    graphics->setFont(getFont());

    for (int i = 0; i < mMaxItems; i++)
    {
        const int itemX = (i % mGridWidth) * mBoxWidth;
        const int itemY = (i / mGridWidth) * mBoxHeight;

        g->drawImage(mBackgroundImg, itemX, itemY);

        // Draw item keyboard shortcut.
        const char *key = SDL_GetKeyName(
            (SDLKey) keyboard.getKeyValue(keyboard.KEY_SHORTCUT_1 + i));
        graphics->setColor(0x000000);
        g->drawText(key, itemX + 2, itemY + 2, gcn::Graphics::LEFT);

        if (itemShortcut->getItem(i) < 0)
            continue;

        Item *item =
            player_node->getInventory()->findItem(itemShortcut->getItem(i));
        if (item) {
            // Draw item icon.
            const std::string label =
                item->isEquipped() ? "Eq." : toString(item->getQuantity());
            Image* image = item->getImage();
            if (image) {
                const std::string label =
                    item->isEquipped() ? "Eq." : toString(item->getQuantity());
                g->drawImage(image, itemX, itemY);
                g->drawText(
                        label,
                        itemX + mBoxWidth / 2,
                        itemY + mBoxHeight - 14,
                        gcn::Graphics::CENTER);
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
            g->drawText(
                    toString(mItemMoved->getQuantity()),
                    tPosX + mBoxWidth / 2,
                    tPosY + mBoxHeight - 14,
                    gcn::Graphics::CENTER);
        }
    }

    if (config.getValue("guialpha", 0.8) != mAlpha)
    {
        mBackgroundImg->setAlpha(config.getValue("guialpha", 0.8));
    }
}

void ItemShortcutContainer::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (!mItemMoved && mItemClicked)
        {
            const int index = getIndexFromGrid(event.getX(), event.getY());
            const int itemId = itemShortcut->getItem(index);

            if (index == -1)
                return;

            if (itemId < 0)
                return;

            Item *item = player_node->getInventory()->findItem(itemId);

            if (item)
            {
                mItemMoved = item;
                itemShortcut->removeItem(index);
            }
        }
        if (mItemMoved) {
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
        if (itemShortcut->isItemSelected())
        {
            itemShortcut->setItem(index);
            itemShortcut->setItemSelected(-1);
        }
        else if (itemShortcut->getItem(index))
            mItemClicked = true;
    }
    else if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Item *item = player_node->getInventory()->
                     findItem(itemShortcut->getItem(index));

        if (!item)
            return;

        /* Convert relative to the window coordinates to absolute screen
         * coordinates.
         */
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        viewport->showPopup(mx, my, item);
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
            mItemMoved = NULL;
            return;
        }
        if (mItemMoved)
        {
            itemShortcut->setItems(index, mItemMoved->getId());
            mItemMoved = NULL;
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
    const int index = getIndexFromGrid(event.getX(), event.getY());
    const int itemId = itemShortcut->getItem(index);

    if (index == -1)
         return;

    if (itemId < 0)
        return;

    Item *item = player_node->getInventory()->findItem(itemId);

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
void ItemShortcutContainer::mouseExited(gcn::MouseEvent &event)
{
    mItemPopup->setVisible(false);
}

