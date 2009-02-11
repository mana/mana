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

#include <guichan/font.hpp>
#include <guichan/listmodel.hpp>

#include "color.h"
#include "shop.h"
#include "shoplistbox.h"

#include "../configuration.h"
#include "../graphics.h"

const int ITEM_ICON_SIZE = 32;

float ShopListBox::mAlpha = config.getValue("guialpha", 0.8);

ShopListBox::ShopListBox(gcn::ListModel *listModel):
    ListBox(listModel),
    mPlayerMoney(0)
{
    mRowHeight = getFont()->getHeight();
    mPriceCheck = true;
}

ShopListBox::ShopListBox(gcn::ListModel *listModel, ShopItems *shopListModel):
    ListBox(listModel),
    mPlayerMoney(0),
    mShopItems(shopListModel)
{
    mRowHeight = std::max(getFont()->getHeight(), ITEM_ICON_SIZE);
    mPriceCheck = true;
}

void ShopListBox::setPlayersMoney(int money)
{
    mPlayerMoney = money;
}

void ShopListBox::draw(gcn::Graphics *gcnGraphics)
{
    if (!mListModel)
        return;

    if (config.getValue("guialpha", 0.8) != mAlpha)
        mAlpha = config.getValue("guialpha", 0.8);

    bool valid;
    const int red = (textColor->getColor('H', valid) >> 16) & 0xFF;
    const int green = (textColor->getColor('H', valid) >> 8) & 0xFF;
    const int blue = textColor->getColor('H', valid) & 0xFF;
    const int alpha = (int)(mAlpha * 255.0f);

    Graphics *graphics = static_cast<Graphics*>(gcnGraphics);

    graphics->setFont(getFont());

    // Draw the list elements
    for (int i = 0, y = 0;
         i < mListModel->getNumberOfElements();
         ++i, y += mRowHeight)
    {
        gcn::Color backgroundColor = gcn::Color(255, 255, 255, alpha);

        if (i == mSelected)
        {
            backgroundColor = gcn::Color(red, green, blue, alpha);
        }
        else if (mShopItems &&
                mPlayerMoney < mShopItems->at(i)->getPrice() && mPriceCheck)
        {
            backgroundColor = gcn::Color(145, 145, 145, alpha);
        }

        graphics->setColor(backgroundColor);
        graphics->fillRectangle(gcn::Rectangle(0, y, getWidth(), mRowHeight));

        if (mShopItems)
        {
            Image *icon = mShopItems->at(i)->getImage();
            if (icon)
            {
                graphics->drawImage(icon, 1, y);
            }
        }
        graphics->setColor(gcn::Color(0, 0, 0));
        graphics->drawText(mListModel->getElementAt(i), ITEM_ICON_SIZE + 5,
                y + (ITEM_ICON_SIZE - getFont()->getHeight()) / 2);
    }
}

void ShopListBox::mousePressed(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        setSelected(event.getY() / mRowHeight);
        distributeActionEvent();
    }
}

void ShopListBox::adjustSize()
{
    if (mListModel)
    {
        setHeight(mRowHeight * mListModel->getNumberOfElements());
    }
}

void ShopListBox::setPriceCheck(bool check)
{
    mPriceCheck = check;
}
