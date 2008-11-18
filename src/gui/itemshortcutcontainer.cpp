/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "itemshortcutcontainer.h"

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
    mGridWidth(1),
    mGridHeight(1),
    mItemClicked(false),
    mItemMoved(NULL)
{
    addMouseListener(this);
    addWidgetListener(this);

    ResourceManager *resman = ResourceManager::getInstance();

    mBackgroundImg = resman->getImage("graphics/gui/item_shortcut_bgr.png");
    mMaxItems = itemShortcut->getItemCount();

    mBoxHeight = 42;
    mBoxWidth = 36;
}

ItemShortcutContainer::~ItemShortcutContainer()
{
    mBackgroundImg->decRef();
}

void
ItemShortcutContainer::logic()
{
    gcn::Widget::logic();

    int i = itemShortcut->getItemCount();

    if (i != mMaxItems)
    {
        mMaxItems = i;
        setWidth(getWidth());
    }
}

void
ItemShortcutContainer::draw(gcn::Graphics *graphics)
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
            (SDLKey) keyboard.getKeyValue(keyboard.KEY_SHORTCUT_0+i));
        g->drawText(key, itemX + 2, itemY + 2, gcn::Graphics::LEFT);

        if (itemShortcut->getItem(i) < 0)
            continue;

        Item *item =
            player_node->getInventory()->findItem(itemShortcut->getItem(i));
        if (item) {
            // Draw item icon.
            Image* image = item->getImage();
            if (image) {
                g->drawImage(image, itemX, itemY);
                g->drawText(
                        toString(item->getQuantity()),
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
}

void ItemShortcutContainer::widgetResized(const gcn::Event &event)
{
    mGridWidth = getWidth() / mBoxWidth;
    if (mGridWidth < 1) {
        mGridWidth = 1;
    }

    setHeight((mMaxItems / mGridWidth +
                (mMaxItems % mGridWidth > 0 ? 1 : 0)) * mBoxHeight);

    mGridHeight = getHeight() / mBoxHeight;
    if (mGridHeight < 1) {
        mGridHeight = 1;
    }
}

void
ItemShortcutContainer::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT) {
        if (!mItemMoved && mItemClicked) {
            const int index = getIndexFromGrid(event.getX(), event.getY());
            if (index == -1) {
                return;
            }
            const int itemId = itemShortcut->getItem(index);
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

void
ItemShortcutContainer::mousePressed(gcn::MouseEvent &event)
{
    const int index = getIndexFromGrid(event.getX(), event.getY());
    if (index == -1) {
        return;
    }

    // Stores the selected item if theirs one.
    if (itemShortcut->isItemSelected()) {
        itemShortcut->setItem(index);
        itemShortcut->setItemSelected(-1);
    }
    else if (itemShortcut->getItem(index)) {
        mItemClicked = true;
    }
}

void
ItemShortcutContainer::mouseReleased(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (itemShortcut->isItemSelected())
        {
            itemShortcut->setItemSelected(-1);
        }
        const int index = getIndexFromGrid(event.getX(), event.getY());
        if (index == -1) {
            mItemMoved = NULL;
            return;
        }
        if (mItemMoved) {
            itemShortcut->setItems(index, mItemMoved->getId());
            mItemMoved = NULL;
        }
        else if (itemShortcut->getItem(index) && mItemClicked)
        {
            itemShortcut->useItem(index);
        }
        if (mItemClicked) {
            mItemClicked = false;
        }
    }
}

int
ItemShortcutContainer::getIndexFromGrid(int pointX, int pointY) const
{
    const gcn::Rectangle tRect = gcn::Rectangle(
        0, 0, mGridWidth * mBoxWidth, mGridHeight * mBoxHeight);
    if (!tRect.isPointInRect(pointX, pointY)) {
        return -1;
    }
    const int index = ((pointY / mBoxHeight) * mGridWidth) +
        pointX / mBoxWidth;
    if (index >= mMaxItems)
    {
        return -1;
    }
    return index;
}
