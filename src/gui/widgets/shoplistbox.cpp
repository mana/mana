/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "gui/widgets/shoplistbox.h"

#include "configuration.h"
#include "graphics.h"
#include "shopitem.h"

#include "gui/itempopup.h"
#include "gui/viewport.h"

#include "gui/widgets/shopitems.h"

#include "resources/image.h"
#include "resources/theme.h"

#include <guichan/font.hpp>
#include <guichan/listmodel.hpp>

float ShopListBox::mAlpha = 1.0;

ShopListBox::ShopListBox(gcn::ListModel *listModel):
    ListBox(listModel),
    mPlayerMoney(0)
{
    mRowHeight = getFont()->getHeight();
    mPriceCheck = true;

    mItemPopup = new ItemPopup;
}

ShopListBox::ShopListBox(gcn::ListModel *listModel, ShopItems *shopListModel):
    ListBox(listModel),
    mPlayerMoney(0),
    mShopItems(shopListModel)
{
    mRowHeight = std::max(getFont()->getHeight(), ITEM_ICON_SIZE);
    mPriceCheck = true;

    mItemPopup = new ItemPopup;
}

ShopListBox::~ShopListBox()
{
    delete mItemPopup;
}

void ShopListBox::setPlayersMoney(int money)
{
    mPlayerMoney = money;
}

void ShopListBox::draw(gcn::Graphics *gcnGraphics)
{
    if (!mListModel)
        return;

    if (config.getFloatValue("guialpha") != mAlpha)
        mAlpha = config.getFloatValue("guialpha");

    const int alpha = (int)(mAlpha * 255.0f);
    const gcn::Color &highlightColor =
            Theme::getThemeColor(Theme::HIGHLIGHT, alpha);
    const gcn::Color &backgroundColor =
            Theme::getThemeColor(Theme::BACKGROUND, alpha);
    const gcn::Color &warningColor =
            Theme::getThemeColor(Theme::SHOP_WARNING, alpha);
    const gcn::Color &textColor =
            Theme::getThemeColor(Theme::TEXT);

    Graphics *graphics = static_cast<Graphics*>(gcnGraphics);

    graphics->setFont(getFont());
    const int fontHeight = getFont()->getHeight();

    // Draw the list elements
    for (int i = 0, y = 0;
         i < mListModel->getNumberOfElements();
         ++i, y += mRowHeight)
    {
        ShopItem *shopItem = mShopItems ? mShopItems->at(i) : 0;

        if (shopItem && mPlayerMoney < shopItem->getPrice() && mPriceCheck)
        {
            if (i != mSelected)
            {
                graphics->setColor(warningColor);
            }
            else
            {
                gcn::Color blend = warningColor;
                blend.r = (blend.r + highlightColor.r) / 2;
                blend.g = (blend.g + highlightColor.g) / 2;
                blend.b = (blend.g + highlightColor.b) / 2;
                graphics->setColor(blend);
            }
        }
        else if (i == mSelected)
        {
            graphics->setColor(highlightColor);
        }
        else
        {
            graphics->setColor(backgroundColor);
        }

        graphics->fillRectangle(gcn::Rectangle(0, y, getWidth(), mRowHeight));

        if (shopItem)
        {
            if (Image *icon = shopItem->getImage())
            {
                icon->setAlpha(1.0f);
                graphics->drawImage(icon, 1, y);
            }

            // Draw the item quantity when it's not just a single item
            if (shopItem->getQuantity() > 1)
            {
                graphics->setColor(textColor);
                graphics->drawText(toString(shopItem->getQuantity()),
                                   1 + ITEM_ICON_SIZE,
                                   y + ITEM_ICON_SIZE - fontHeight,
                                   Graphics::RIGHT);
            }
        }

        graphics->setColor(textColor);
        graphics->drawText(mListModel->getElementAt(i),
                           ITEM_ICON_SIZE + 5,
                           y + (ITEM_ICON_SIZE - fontHeight) / 2);
    }
}

void ShopListBox::adjustSize()
{
    if (mListModel)
        setHeight(mRowHeight * mListModel->getNumberOfElements());
}

void ShopListBox::setPriceCheck(bool check)
{
    mPriceCheck = check;
}

void ShopListBox::mouseMoved(gcn::MouseEvent &event)
{
    if (!mShopItems)
        return;

    int index = event.getY() / mRowHeight;

    if (index < 0 || index >= mShopItems->getNumberOfElements())
    {
        mItemPopup->setVisible(false);
    }
    else
    {
        Item *item = mShopItems->at(index);
        if (item)
        {
            mItemPopup->setItem(item->getInfo());
            mItemPopup->position(viewport->getMouseX(), viewport->getMouseY());
        }
        else
        {
            mItemPopup->setVisible(false);
        }
    }
}

void ShopListBox::mouseExited(gcn::MouseEvent &event)
{
    mItemPopup->setVisible(false);
}

