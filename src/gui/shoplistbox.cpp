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
 *  $Id: listbox.cpp 2655 2006-09-03 21:25:02Z b_lindeijer $
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

const int ITEM_SPRITE_HEIGHT = 32;

ShopListBox::ShopListBox(gcn::ListModel *listModel):
    gcn::ListBox(listModel),
    mMousePressed(false),
    mPlayerMoney(0)
{
    mRowHeight = getFont()->getHeight();
    mPriceCheck = true;
}

ShopListBox::ShopListBox(gcn::ListModel *listModel, ShopItems *shopListModel):
    gcn::ListBox(listModel),
    mMousePressed(false),
    mPlayerMoney(0),
    mShopItems(shopListModel)
{
    mRowHeight = (getFont()->getHeight() > ITEM_SPRITE_HEIGHT ?
        getFont()->getHeight() : ITEM_SPRITE_HEIGHT);
    mPriceCheck = true;
}


void ShopListBox::setPlayersMoney(int money)
{
    mPlayerMoney = money;
}

void ShopListBox::draw(gcn::Graphics *graphics)
{
    if (mListModel == NULL) {
        return;
    }

    graphics->setFont(getFont());

    // Draw the list elements
    for (int i = 0, y = 0; i < mListModel->getNumberOfElements(); ++i, y += mRowHeight)
    {
        graphics->setColor(gcn::Color(0xffffff));
        if (mShopItems)
        {
            if(mPlayerMoney < mShopItems->at(i).price && mPriceCheck)
            {
                graphics->setColor(gcn::Color(0x919191));
            }
        }
        graphics->fillRectangle(gcn::Rectangle(0, y, getWidth(), mRowHeight));
        if (mShopItems)
            dynamic_cast<Graphics*>(graphics)->drawImage(mShopItems->at(i).image, 1, y);
        graphics->drawText(mListModel->getElementAt(i), ITEM_SPRITE_HEIGHT, y);
    }

    // Draw rectangle below the selected list element and the list element
    // not shown.
    if (mSelected >= 0) {
        graphics->setColor(gcn::Color(110, 160, 255));
        graphics->fillRectangle(gcn::Rectangle(0, mRowHeight * mSelected,
                                               getWidth(), mRowHeight));
        if (mShopItems)
            dynamic_cast<Graphics*>(graphics)->drawImage(
            mShopItems->at(mSelected).image, 1, mRowHeight * mSelected);
        graphics->drawText(mListModel->getElementAt(mSelected),
            ITEM_SPRITE_HEIGHT, mRowHeight * mSelected);
    }
}

void ShopListBox::setSelected(int selected)
{
    gcn::ListBox::setSelected(selected);
    if (mListModel != NULL)
    {
        gcn::BasicContainer *par = getParent();
        if (par == NULL)
        {
            return;
        }

        gcn::Rectangle scroll;

        if (mSelected < 0)
        {
            scroll.y = 0;
        }
        else
        {
            scroll.y = mRowHeight * mSelected;
        }

        scroll.height = mRowHeight;
        par->showWidgetPart(this, scroll);
    }
    fireSelectionChangedEvent();
}

void ShopListBox::mousePress(int x, int y, int button)
{

    bool enoughMoney = false;
    if (button == gcn::MouseInput::LEFT && hasMouse())
    {
        if (mShopItems)
        {
            if(mPlayerMoney >= mShopItems->at(y / mRowHeight).price)
                enoughMoney = true;
        }
        else // Old Behaviour
            enoughMoney = true;

    if (!mPriceCheck)
        enoughMoney = true;

        if (enoughMoney)
        {
            setSelected(y / mRowHeight);
            generateAction();
            mMousePressed = true;
        }
    }
}

void ShopListBox::mouseRelease(int x, int y, int button)
{
    gcn::ListBox::mouseRelease(x, y, button);

    mMousePressed = false;
}

void ShopListBox::mouseMotion(int x, int y)
{
    gcn::ListBox::mouseMotion(x, y);

    // Pretend mouse is pressed continuously while dragged. Causes list
    // selection to be updated as is default in many GUIs.
    if (mMousePressed)
    {
        mousePress(x, y, gcn::MouseInput::LEFT);
    }
}

void ShopListBox::fireSelectionChangedEvent()
{
    SelectionEvent event(this);
    SelectionListeners::iterator i_end = mListeners.end();
    SelectionListeners::iterator i;

    for (i = mListeners.begin(); i != i_end; ++i)
    {
        (*i)->selectionChanged(event);
    }
}

void ShopListBox::adjustSize()
{
    if (mListModel != NULL)
    {
        setHeight(mRowHeight * mListModel->getNumberOfElements());
    }
}

void ShopListBox::setPriceCheck(bool check)
{
    mPriceCheck = check;
}
