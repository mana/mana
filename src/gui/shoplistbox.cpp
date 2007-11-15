/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
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
 *
 *  $Id$
 */

#include "shoplistbox.h"

#include "selectionlistener.h"

#include <guichan/font.hpp>
#include <guichan/graphics.hpp>
#include <guichan/listmodel.hpp>
#include <guichan/mouseinput.hpp>
#include <guichan/imagefont.hpp>
#include <guichan/basiccontainer.hpp>

#include "../graphics.h"

const int ITEM_ICON_SIZE = 32;

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

    Graphics *graphics = static_cast<Graphics*>(gcnGraphics);

    graphics->setFont(getFont());

    // Draw the list elements
    for (int i = 0, y = 0;
         i < mListModel->getNumberOfElements();
         ++i, y += mRowHeight)
    {
        gcn::Color backgroundColor = gcn::Color(0xffffff);

        if (i == mSelected)
        {
            backgroundColor = gcn::Color(110, 160, 255);
        }
        else if (mShopItems &&
                mPlayerMoney < mShopItems->at(i)->getPrice() && mPriceCheck)
        {
            backgroundColor = gcn::Color(0x919191);
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

void ShopListBox::setSelected(int selected)
{
    if (!mListModel)
    {
        mSelected = -1;
    }
    else
    {
        // Update mSelected with bounds checking
        mSelected = std::min(mListModel->getNumberOfElements() - 1,
                             std::max(-1, selected));

        gcn::Widget *parent = getParent();
        if (parent)
        {
            gcn::Rectangle scroll;
            scroll.y = (mSelected < 0) ? 0 : mRowHeight * mSelected;
            scroll.height = mRowHeight;
            parent->showWidgetPart(this, scroll);
        }
    }

    fireSelectionChangedEvent();
}

void ShopListBox::mousePressed(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        setSelected(event.getY() / mRowHeight);
        generateAction();
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
