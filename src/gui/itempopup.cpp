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

#include "configuration.h"
#include "units.h"

#include "gui/gui.h"

#include "gui/widgets/icon.h"
#include "gui/widgets/label.h"
#include "gui/widgets/textbox.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include "resources/resourcemanager.h"
#include "resources/theme.h"

#include <guichan/font.hpp>

#include <guichan/widgets/label.hpp>

#define ITEMPOPUP_WRAP_WIDTH 196

static const gcn::Color &getColorFromItemType(ItemType type)
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
    Popup("ItemPopup")
{
    setMinHeight(boldFont->getHeight());

    // Item Name
    mItemName = new Label;
    mItemName->setFont(boldFont);
    mItemName->setPosition(getPadding(), getPadding());

    // Item Description
    mItemDesc = new TextBox;
    mItemDesc->setEditable(false);

    // Item Effect
    mItemEffect = new TextBox;
    mItemEffect->setEditable(false);

    // Item Weight
    mItemWeight = new TextBox;
    mItemWeight->setEditable(false);

    mIcon = new Icon;

    add(mItemName);
    add(mItemDesc);
    add(mItemEffect);
    add(mItemWeight);
    add(mIcon);

    addMouseListener(this);
}

ItemPopup::~ItemPopup() = default;

void ItemPopup::setEquipmentText(const std::string& text)
{
    mItemEquipSlot = text;
}

void ItemPopup::setNoItem()
{
    mIcon->setImage(nullptr);

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
    mItemWeight->setText(std::string());

    setContentSize(mItemName->getWidth(), mItemName->getHeight());
}

void ItemPopup::setItem(const ItemInfo &item, bool showImage)
{
    if (item.name == mItemName->getCaption())
        return;

    int space = 0;

    if (showImage)
    {
        ResourceManager *resman = ResourceManager::getInstance();
        auto image = resman->getImage(paths.getStringValue("itemIcons") +
                                      item.display.image);

        mIcon->setImage(image);
        if (image)
        {
            mIcon->setPosition(0, 0);
            space = mIcon->getWidth();
        }
    }
    else
    {
        mIcon->setImage(nullptr);
    }

    mItemType = item.type;

    std::string caption = item.name;
    if (!mItemEquipSlot.empty())
        caption += " (" + mItemEquipSlot + ")";

    mItemName->setCaption(caption);
    mItemName->adjustSize();
    mItemName->setForegroundColor(getColorFromItemType(mItemType));
    mItemName->setPosition(space, 0);

    mItemDesc->setTextWrapped(item.description, ITEMPOPUP_WRAP_WIDTH);
    mItemEffect->setTextWrapped(join(item.effect, "\n"), ITEMPOPUP_WRAP_WIDTH);
    mItemWeight->setTextWrapped(strprintf(_("Weight: %s"),
                                Units::formatWeight(item.weight).c_str()),
                                ITEMPOPUP_WRAP_WIDTH);

    int minWidth = mItemName->getWidth() + space;

    if (mItemDesc->getMinWidth() > minWidth)
        minWidth = mItemDesc->getMinWidth();
    if (mItemEffect->getMinWidth() > minWidth)
        minWidth = mItemEffect->getMinWidth();
    if (mItemWeight->getMinWidth() > minWidth)
        minWidth = mItemWeight->getMinWidth();

    const int descHeight = mItemDesc->getHeight();
    const int effectHeight = mItemEffect->getHeight();
    const int weightHeight = mItemWeight->getHeight();

    int nameHeight = std::max(mItemName->getHeight(), mIcon->getHeight());
    nameHeight += getPadding();

    if (item.effect.empty())
    {
        setContentSize(minWidth, nameHeight + descHeight + weightHeight + getPadding());

        mItemWeight->setPosition(0, nameHeight + descHeight + getPadding());
    }
    else
    {
        setContentSize(minWidth, nameHeight + descHeight + effectHeight +
                       weightHeight + getPadding());

        mItemWeight->setPosition(0, nameHeight + descHeight + effectHeight +
                                 getPadding());
    }

    mItemDesc->setPosition(0, nameHeight);
    mItemEffect->setPosition(0, nameHeight + descHeight + getPadding());
}

void ItemPopup::mouseMoved(gcn::MouseEvent &event)
{
    Popup::mouseMoved(event);

    // When the mouse moved on top of the popup, hide it
    setVisible(false);
    mItemEquipSlot.clear();
}
