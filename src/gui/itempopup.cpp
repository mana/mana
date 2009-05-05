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

#include "gui/itempopup.h"

#include "gui/gui.h"
#include "gui/palette.h"

#include "gui/widgets/textbox.h"

#include "graphics.h"
#include "units.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>
#include <guichan/widgets/label.hpp>

ItemPopup::ItemPopup():
    Popup("ItemPopup")
{
    // Item Name
    mItemName = new gcn::Label;
    mItemName->setFont(boldFont);
    mItemName->setPosition(getPadding(), getPadding());

    const int fontHeight = getFont()->getHeight();

    // Item Description
    mItemDesc = new TextBox;
    mItemDesc->setEditable(false);
    mItemDesc->setPosition(getPadding(), fontHeight);

    // Item Effect
    mItemEffect = new TextBox;
    mItemEffect->setEditable(false);
    mItemEffect->setPosition(getPadding(), 2 * fontHeight + 2 * getPadding());

    // Item Weight
    mItemWeight = new TextBox;
    mItemWeight->setEditable(false);
    mItemWeight->setPosition(getPadding(), 3 * fontHeight + 4 * getPadding());

    add(mItemName);
    add(mItemDesc);
    add(mItemEffect);
    add(mItemWeight);

    addMouseListener(this);

    loadPopupConfiguration();
}

ItemPopup::~ItemPopup()
{
}

void ItemPopup::setItem(const ItemInfo &item)
{
    if (item.getName() == mItemName->getCaption())
        return;

    mItemType = item.getType();

    mItemName->setCaption(item.getName());
    mItemName->adjustSize();
    mItemName->setForegroundColor(getColor(mItemType));

    mItemDesc->setTextWrapped(item.getDescription(), 196);
    mItemEffect->setTextWrapped(item.getEffect(), 196);
    mItemWeight->setTextWrapped(_("Weight: ") +
                                Units::formatWeight(item.getWeight()), 196);

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

    if (item.getEffect().empty())
    {
        setContentSize(minWidth, (numRowsDesc + numRowsWeight + getPadding()) *
                       height);

        mItemWeight->setPosition(getPadding(), (numRowsDesc + getPadding()) *
                                 height);
    }
    else
    {
        setContentSize(minWidth, (numRowsDesc + numRowsEffect + numRowsWeight +
                       getPadding()) * height);

        mItemWeight->setPosition(getPadding(), (numRowsDesc + numRowsEffect +
                                 getPadding()) * height);
    }

    mItemDesc->setPosition(getPadding(), 2 * height);
    mItemEffect->setPosition(getPadding(), (numRowsDesc + getPadding()) * height);
}

gcn::Color ItemPopup::getColor(ItemType type)
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

void ItemPopup::mouseMoved(gcn::MouseEvent &event)
{
    // When the mouse moved on top of the popup, hide it
    setVisible(false);
}
