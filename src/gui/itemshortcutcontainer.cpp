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

#include "gui/itemshortcutcontainer.h"

#include "gui/inventorywindow.h"
#include "gui/itempopup.h"
#include "gui/palette.h"
#include "gui/viewport.h"

#include "configuration.h"
#include "graphics.h"
#include "inventory.h"
#include "item.h"
#include "itemshortcut.h"
#include "keyboardconfig.h"
#include "localplayer.h"

#include "resources/image.h"
#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"

#include "utils/stringutils.h"

ItemShortcutContainer::ItemShortcutContainer():
    ShortcutContainer(),
    mItemClicked(false),
    mItemMoved(NULL)
{
    addMouseListener(this);
    addWidgetListener(this);

    mItemPopup = new ItemPopup;

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

void ItemShortcutContainer::draw(gcn::Graphics *graphics)
{
    if (config.getValue("guialpha", 0.8) != mAlpha)
    {
        mAlpha = config.getValue("guialpha", 0.8);
        mBackgroundImg->setAlpha(mAlpha);
    }

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
        graphics->setColor(guiPalette->getColor(Palette::TEXT));
        g->drawText(key, itemX + 2, itemY + 2, gcn::Graphics::LEFT);

        if (itemShortcut->getItem(i) < 0)
            continue;

        Item *item =
            player_node->getInventory()->findItem(itemShortcut->getItem(i));

        if (item)
        {
            // Draw item icon.
            Image* image = item->getImage();

            if (image)
            {
                std::string caption;
                if (item->getQuantity() > 1)
                    caption = toString(item->getQuantity());
                else if (item->isEquipped())
                    caption = "Eq.";

                g->drawImage(image, itemX, itemY);
                if (item->isEquipped())
                    g->setColor(guiPalette->getColor(Palette::ITEM_EQUIPPED));
                g->drawText(caption, itemX + mBoxWidth / 2,
                            itemY + mBoxHeight - 14, gcn::Graphics::CENTER);
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
            const int itemId = itemShortcut->getItem(index);

            if (index == -1 || itemId < 0)
                return;

            Item *item = player_node->getInventory()->findItem(itemId);

            if (item)
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
        Item *item = player_node->getInventory()->
                     findItem(itemShortcut->getItem(index));

        if (!item)
            return;

        // Convert relative to the window coordinates to absolute screen
        // coordinates.
        viewport->showPopup(viewport->getMouseX(), viewport->getMouseY(), item);
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

    if (index == -1 || itemId < 0)
        return;

    Item *item = player_node->getInventory()->findItem(itemId);

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
void ItemShortcutContainer::mouseExited(gcn::MouseEvent &event)
{
    mItemPopup->setVisible(false);
}
