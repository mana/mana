/*
 *  The Mana World
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008  The Mana World Development Team
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

#include <guichan/widgets/label.hpp>

#include "gui.h"
#include "itempopup.h"
#include "palette.h"
#include "scrollarea.h"
#include "textbox.h"

#include "../graphics.h"

#include "../units.h"

#include "../resources/iteminfo.h"

#include "../utils/gettext.h"
#include "../utils/stringutils.h"

ItemPopup::ItemPopup():
    Popup("ItemPopup")
{
    // Item Name
    mItemName = new gcn::Label;
    mItemName->setFont(boldFont);
    mItemName->setPosition(2, 2);

    // Item Description
    mItemDesc = new TextBox;
    mItemDesc->setEditable(false);
    mItemDescScroll = new ScrollArea(mItemDesc);

    mItemDescScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemDescScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemDescScroll->setDimension(gcn::Rectangle(0, 0, 196, getFont()->getHeight()));
    mItemDescScroll->setOpaque(false);
    mItemDescScroll->setPosition(2, getFont()->getHeight());

    // Item Effect
    mItemEffect = new TextBox;
    mItemEffect->setEditable(false);
    mItemEffectScroll = new ScrollArea(mItemEffect);

    mItemEffectScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemEffectScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemEffectScroll->setDimension(gcn::Rectangle(0, 0, 196, getFont()->getHeight()));
    mItemEffectScroll->setOpaque(false);
    mItemEffectScroll->setPosition(2, (2 * getFont()->getHeight()) + 
                                  (2 * getPadding()));

    // Item Weight
    mItemWeight = new TextBox;
    mItemWeight->setEditable(false);
    mItemWeightScroll = new ScrollArea(mItemWeight);

    mItemWeightScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemWeightScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemWeightScroll->setDimension(gcn::Rectangle(0, 0, 196, getFont()->getHeight()));
    mItemWeightScroll->setOpaque(false);
    mItemWeightScroll->setPosition(2, (3 * getFont()->getHeight()) + 
                                  (4 * getPadding()));

    add(mItemName);
    add(mItemDescScroll);
    add(mItemEffectScroll);
    add(mItemWeightScroll);

    loadPopupConfiguration();
}

ItemPopup::~ItemPopup()
{
    delete mItemName;
    delete mItemDesc;
    delete mItemDescScroll;
    delete mItemEffect;
    delete mItemEffectScroll;
    delete mItemWeight;
    delete mItemWeightScroll;
}

void ItemPopup::setItem(const ItemInfo &item)
{
    if (item.getName() == mItemName->getCaption())
        return;

    mItemName->setCaption(item.getName());
    mItemName->setWidth(boldFont->getWidth(item.getName()));
    mItemDesc->setTextWrapped(item.getDescription(), 196);
    mItemEffect->setTextWrapped(item.getEffect(), 196);
    mItemWeight->setTextWrapped(_("Weight: ") +
                                Units::formatWeight(item.getWeight()), 196);
    mItemType = item.getType();

    int minWidth = mItemName->getWidth();

    if (mItemDesc->getMinWidth() > minWidth)
        minWidth = mItemDesc->getMinWidth();
    if (mItemEffect->getMinWidth() > minWidth)
        minWidth = mItemEffect->getMinWidth();
    if (mItemWeight->getMinWidth() > minWidth)
        minWidth = mItemWeight->getMinWidth();

    minWidth += 8;
    setWidth(minWidth);

    const int numRowsDesc = mItemDesc->getNumberOfRows();
    const int numRowsEffect = mItemEffect->getNumberOfRows();
    const int numRowsWeight = mItemWeight->getNumberOfRows();
    const int height = getFont()->getHeight();

    mItemDescScroll->setDimension(gcn::Rectangle(2, 0, minWidth, numRowsDesc * 
                                                 height));

    mItemEffectScroll->setDimension(gcn::Rectangle(2, 0, minWidth, numRowsEffect
                                                   * height));

    mItemWeightScroll->setDimension(gcn::Rectangle(2, 0, minWidth, numRowsWeight
                                                   * height));

    if (item.getEffect().empty())
    {
        setContentSize(minWidth, (numRowsDesc + 3) * height);

        mItemWeightScroll->setPosition(2, (numRowsDesc + 2) * height);
    }
    else
    {
        setContentSize(minWidth, (numRowsDesc + numRowsEffect + 3) * height);

        mItemWeightScroll->setPosition(2, (numRowsDesc + numRowsEffect + 2) *
                                       height);
    }

    mItemDescScroll->setPosition(2, 20);
    mItemEffectScroll->setPosition(2, (numRowsDesc + 2) * height);
}

void ItemPopup::updateColors()
{
    mItemName->setForegroundColor(getColor(mItemType));
    graphics->setColor(guiPalette->getColor(Palette::TEXT));
}

gcn::Color ItemPopup::getColor(short type)
{
    switch (type)
    {
        case ITEM_UNUSABLE:
            return guiPalette->getColor(Palette::GENERIC);
        case ITEM_USABLE:
            return guiPalette->getColor(Palette::USABLE);
        case ITEM_EQUIPMENT_ONE_HAND_WEAPON:
            return guiPalette->getColor(Palette::ONEHAND);
        case ITEM_EQUIPMENT_TWO_HANDS_WEAPON:
            return guiPalette->getColor(Palette::TWOHAND);
        case ITEM_EQUIPMENT_TORSO:
            return guiPalette->getColor(Palette::TORSO);
        case ITEM_EQUIPMENT_ARMS:
            return guiPalette->getColor(Palette::ARMS);
        case ITEM_EQUIPMENT_HEAD:
            return guiPalette->getColor(Palette::HEAD);
        case ITEM_EQUIPMENT_LEGS:
            return guiPalette->getColor(Palette::LEGS);
        case ITEM_EQUIPMENT_SHIELD:
            return guiPalette->getColor(Palette::SHIELD);
        case ITEM_EQUIPMENT_RING:
            return guiPalette->getColor(Palette::RING);
        case ITEM_EQUIPMENT_NECKLACE:
            return guiPalette->getColor(Palette::NECKLACE);
        case ITEM_EQUIPMENT_FEET:
            return guiPalette->getColor(Palette::FEET);
        case ITEM_EQUIPMENT_AMMO:
            return guiPalette->getColor(Palette::AMMO);
        default:
            return guiPalette->getColor(Palette::UNKNOWN_ITEM);
    }
}

std::string ItemPopup::getItemName() const
{
    return mItemName->getCaption();
}

unsigned int ItemPopup::getNumRows()
{
    return mItemDesc->getNumberOfRows() + mItemEffect->getNumberOfRows() +
           mItemWeight->getNumberOfRows();
}

void ItemPopup::view(int x, int y)
{
    const int distance = 20;

    int posX = std::max(0, x - getWidth() / 2);
    int posY = y + distance;

    if (posX > graphics->getWidth() - getWidth())
        posX = graphics->getWidth() - getWidth();
    if (posY > graphics->getHeight() - getHeight())
        posY = y - getHeight() - distance;

    setPosition(posX, posY);
    setVisible(true);
    requestMoveToTop();
}
