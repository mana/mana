/*
 *  The Mana Client
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "gui/itempopup.h"

#include "graphics.h"
#include "units.h"

#include "gui/gui.h"

#include "gui/widgets/icon.h"
#include "gui/widgets/label.h"
#include "gui/widgets/textbox.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include "net/net.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"
#include "resources/theme.h"

#include <guichan/font.hpp>

#include <guichan/widgets/label.hpp>

#define ITEMPOPUP_WRAP_WIDTH 196

static gcn::Color getColorFromItemType(ItemType type)
{
    switch (type)
    {
        case ITEM_UNUSABLE:
            return Theme::getThemeColor(Theme::GENERIC);
        case ITEM_USABLE:
            return Theme::getThemeColor(Theme::USABLE);
        case ITEM_EQUIPMENT_ONE_HAND_WEAPON:
            return Theme::getThemeColor(Theme::ONEHAND);
        case ITEM_EQUIPMENT_TWO_HANDS_WEAPON:
            return Theme::getThemeColor(Theme::TWOHAND);
        case ITEM_EQUIPMENT_TORSO:
            return Theme::getThemeColor(Theme::TORSO);
        case ITEM_EQUIPMENT_ARMS:
            return Theme::getThemeColor(Theme::ARMS);
        case ITEM_EQUIPMENT_HEAD:
            return Theme::getThemeColor(Theme::HEAD);
        case ITEM_EQUIPMENT_LEGS:
            return Theme::getThemeColor(Theme::LEGS);
        case ITEM_EQUIPMENT_SHIELD:
            return Theme::getThemeColor(Theme::SHIELD);
        case ITEM_EQUIPMENT_RING:
            return Theme::getThemeColor(Theme::RING);
        case ITEM_EQUIPMENT_NECKLACE:
            return Theme::getThemeColor(Theme::NECKLACE);
        case ITEM_EQUIPMENT_FEET:
            return Theme::getThemeColor(Theme::FEET);
        case ITEM_EQUIPMENT_AMMO:
            return Theme::getThemeColor(Theme::AMMO);
        default:
            return Theme::getThemeColor(Theme::UNKNOWN_ITEM);
    }
}

ItemPopup::ItemPopup():
    Popup("ItemPopup"),
    mIcon(0)
{
    // Item Name
    mItemName = new Label;
    mItemName->setFont(boldFont);

    // Item Description
    mItemDesc = new TextBox;
    mItemDesc->setEditable(false);

    // Item Effect
    mItemEffect = new TextBox;
    mItemEffect->setEditable(false);

    // Item Weight
    mItemWeight = new TextBox;
    mItemWeight->setEditable(false);

    mIcon = new Icon(0);

    add(mItemName);
    add(mItemDesc);
    add(mItemEffect);
    add(mItemWeight);
    add(mIcon);

    addMouseListener(this);
}

ItemPopup::~ItemPopup()
{
    if (mIcon)
    {
        Image *image = mIcon->getImage();
        if (image)
            image->decRef();
    }
}

void ItemPopup::setEquipmentText(const std::string& text)
{
    mItemEquipSlot = text;
}

void ItemPopup::setNoItem()
{
    mIcon->setImage(0);

    std::string caption = _("No item");
    if (!mItemEquipSlot.empty())
    {
        caption += " (";
        caption += mItemEquipSlot;
        caption += ")";
    }
    mItemName->setCaption(caption);
    mItemName->adjustSize();

    mItemName->setForegroundColor(Theme::getThemeColor(Theme::GENERIC));
    mItemName->setPosition(0, 0);

    mItemDesc->setText(std::string());
    mItemEffect->setText(std::string());

    setContentSize(mItemName->getWidth(), 0);
}

void ItemPopup::setItem(const ItemInfo &item, bool showImage)
{
    if (item.getName() == mItemName->getCaption())
        return;

    int space = 0;

    Image *oldImage = mIcon->getImage();
    if (oldImage)
        oldImage->decRef();

    if (showImage)
    {
        ResourceManager *resman = ResourceManager::getInstance();
        Image *image = resman->getImage(
                                  paths.getStringValue("itemIcons")
                                  + item.getDisplay().image);

        mIcon->setImage(image);
        if (image)
            space = mIcon->getWidth();
    }
    else
    {
        mIcon->setImage(0);
    }

    mItemType = item.getItemType();

    std::string caption = item.getName();
    if (!mItemEquipSlot.empty())
        caption += " (" + mItemEquipSlot + ")";

    mItemName->setCaption(caption);
    mItemName->adjustSize();
    mItemName->setForegroundColor(getColorFromItemType(mItemType));
    mItemName->setPosition(space, 0);

    mItemDesc->setTextWrapped(item.getDescription(), ITEMPOPUP_WRAP_WIDTH);
    {
        const std::vector<std::string> &effect = item.getEffect();
        std::string temp = "";
        for (std::vector<std::string>::const_iterator it = effect.begin(),
             it_end = effect.end(); it != it_end; ++it)
            temp += temp.empty() ? *it : "\n" + *it;
        mItemEffect->setTextWrapped(temp, ITEMPOPUP_WRAP_WIDTH);
    }
    mItemWeight->setTextWrapped(strprintf(_("Weight: %s"),
                                Units::formatWeight(item.getWeight()).c_str()),
                                ITEMPOPUP_WRAP_WIDTH);

    int minWidth = mItemName->getWidth() + space;

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
    const int fontHeight = getFont()->getHeight();

    int nameHeight;
    if (mIcon->getHeight() > 2 * fontHeight)
        nameHeight = mIcon->getHeight();
    else
        nameHeight = 2 * fontHeight;

    if (item.getEffect().empty())
    {
        setContentSize(minWidth, nameHeight +
                        (numRowsDesc + numRowsWeight + 1) * fontHeight);

        mItemWeight->setPosition(0,
                                nameHeight + (numRowsDesc + 1) * fontHeight);
    }
    else
    {
        setContentSize(minWidth, nameHeight + (numRowsDesc + numRowsEffect +
                        numRowsWeight + 1) * fontHeight);

        mItemWeight->setPosition(0, nameHeight + (numRowsDesc +
                                    numRowsEffect + 1) * fontHeight);
    }

    mItemDesc->setPosition(0, nameHeight);
    mItemEffect->setPosition(0, nameHeight +
                            (numRowsDesc + 1) * fontHeight);
}

void ItemPopup::mouseMoved(gcn::MouseEvent &event)
{
    Popup::mouseMoved(event);

    // When the mouse moved on top of the popup, hide it
    setVisible(false);
    mItemEquipSlot.clear();
}

