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

#include "smileyshortcutcontainer.h"

#include "../graphics.h"
#include "../inventory.h"
#include "../item.h"
#include "../itemshortcut.h"
#include "../keyboardconfig.h"
#include "../localplayer.h"
#include "../log.h"


#include "../resources/image.h"
#include "../resources/resourcemanager.h"

#include "../utils/gettext.h"
#include "../utils/tostring.h"

SmileyShortcutContainer::SmileyShortcutContainer():
    mGridWidth(1),
    mGridHeight(1),
    mItemClicked(false),
    mItemMoved(NULL)
{
    addMouseListener(this);
    addWidgetListener(this);

    ResourceManager *resman = ResourceManager::getInstance();

    mBackgroundImg = resman->getImage("graphics/gui/item_shortcut_bgr.png");
    mSmileyImg = resman->getImageSet("graphics/gui/emotions.png",30,32);
    if (!mSmileyImg) logger->error(_("Unable to load emotions"));

    mMaxItems = 12; 

    mBoxHeight = 42;
    mBoxWidth = 36;
}

SmileyShortcutContainer::~SmileyShortcutContainer()
{
    mBackgroundImg->decRef();
    if (mSmileyImg)
    {
       mSmileyImg->decRef();
       mSmileyImg=NULL;
    }

}

void
SmileyShortcutContainer::draw(gcn::Graphics *graphics)
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
            (SDLKey) keyboard.getKeyValue(keyboard.KEY_SMILEY_1 + i));
        graphics->setColor(0x000000);
        g->drawText(key, itemX + 2, itemY + 2, gcn::Graphics::LEFT);
        static_cast<Graphics*>(graphics)->drawImage(
                    mSmileyImg->get(i), itemX+2, itemY+10);

#if 0
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
#endif
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

void SmileyShortcutContainer::widgetResized(const gcn::Event &event)
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
SmileyShortcutContainer::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT) {
        if (!mItemMoved && mItemClicked) {
            const int index = getIndexFromGrid(event.getX(), event.getY());
            if (index == -1) {
                return;
            }
#if 0
            const int itemId = itemShortcut->getItem(index);
            if (itemId < 0)
                return;
            Item *item = player_node->getInventory()->findItem(itemId);
            if (item)
            {
                mItemMoved = item;
                itemShortcut->removeItem(index);
            }
#endif
        }
        if (mItemMoved) {
            mCursorPosX = event.getX();
            mCursorPosY = event.getY();
        }
    }
}

void
SmileyShortcutContainer::mousePressed(gcn::MouseEvent &event)
{
    const int index = getIndexFromGrid(event.getX(), event.getY());
    if (index == -1) {
        return;
    }
#if 0
    // Stores the selected item if theirs one.
    if (itemShortcut->isItemSelected()) {
        itemShortcut->setItem(index);
        itemShortcut->setItemSelected(-1);
    }
    else if (itemShortcut->getItem(index)) {
        mItemClicked = true;
    }
#endif
}

void
SmileyShortcutContainer::mouseReleased(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
#if 0
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
#endif
    }
}

int
SmileyShortcutContainer::getIndexFromGrid(int pointX, int pointY) const
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
