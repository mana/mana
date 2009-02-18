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
#include "scrollarea.h"
#include "textbox.h"
#include "windowcontainer.h"

#include "widgets/layout.h"

#include "../units.h"

#include "../resources/iteminfo.h"

#include "../utils/gettext.h"
#include "../utils/stringutils.h"

ItemPopup::ItemPopup():
    Window()
{
    setResizable(false);
    setShowTitle(false);
    setTitleBarHeight(0);

    // Item Name
    mItemName = new gcn::Label("Label");
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
    mItemEffectScroll->setPosition(2, (2 * getFont()->getHeight()) + 5);

    // Item Weight
    mItemWeight = new TextBox;
    mItemWeight->setEditable(false);
    mItemWeightScroll = new ScrollArea(mItemWeight);

    mItemWeightScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemWeightScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemWeightScroll->setDimension(gcn::Rectangle(0, 0, 196, getFont()->getHeight()));
    mItemWeightScroll->setOpaque(false);
    mItemWeightScroll->setPosition(2, (3 * getFont()->getHeight()) + 10);

    add(mItemName);
    add(mItemDescScroll);
    add(mItemEffectScroll);
    add(mItemWeightScroll);

    setLocationRelativeTo(getParent());
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
    mItemName->setCaption(item.getName());
    mItemName->setForegroundColor(getColor(item.getType()));
    mItemName->setWidth(boldFont->getWidth(item.getName()));
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

    int numRowsDesc = mItemDesc->getNumberOfRows();
    int numRowsEffect = mItemEffect->getNumberOfRows();
    int numRowsWeight = mItemWeight->getNumberOfRows();

    mItemDescScroll->setDimension(gcn::Rectangle(2, 0, minWidth,
                                  numRowsDesc * getFont()->getHeight()));

    mItemEffectScroll->setDimension(gcn::Rectangle(2, 0, minWidth,
                                    numRowsEffect * getFont()->getHeight()));

    mItemWeightScroll->setDimension(gcn::Rectangle(2, 0, minWidth,
                                    numRowsWeight * getFont()->getHeight()));

    if (item.getEffect().empty())
    {
        setContentSize(minWidth, (numRowsDesc * getFont()->getHeight() +
                      (3 * getFont()->getHeight())));

        mItemWeightScroll->setPosition(2,
                          (numRowsDesc * getFont()->getHeight()) +
                          (2 * getFont()->getHeight()));
    }
    else
    {
        setContentSize(minWidth, (numRowsDesc * getFont()->getHeight()) +
                      (numRowsEffect * getFont()->getHeight()) +
                      (3 * getFont()->getHeight()));

        mItemWeightScroll->setPosition(2,
                          (numRowsDesc * getFont()->getHeight()) +
                          (numRowsEffect * getFont()->getHeight()) +
                          (2 * getFont()->getHeight()));
    }

    mItemDescScroll->setPosition(2, 20);
    mItemEffectScroll->setPosition(2, (numRowsDesc * getFont()->getHeight()) +
                      (2 * getFont()->getHeight()));
}

gcn::Color ItemPopup::getColor(const std::string& type)
{
    gcn::Color color;

    if (type.compare("generic") == 0)
        color = 0x21a5b1;
    else if (type.compare("equip-head") == 0)
        color = 0x527fa4;
    else if (type.compare("usable") == 0)
        color = 0x268d24;
    else if (type.compare("equip-torso") == 0)
        color = 0xd12aa4;
    else if (type.compare("equip-1hand") == 0)
        color = 0xf42a2a;
    else if (type.compare("equip-legs") == 0)
        color = 0x699900;
    else if (type.compare("equip-feet") == 0)
        color = 0xaa1d48;
    else if (type.compare("equip-2hand") == 0)
        color = 0xf46d0e;
    else if (type.compare("equip-shield") == 0)
        color = 0x9c2424;
    else if (type.compare("equip-ring") == 0)
        color = 0x0000ff;
    else if (type.compare("equip-arms") == 0)
        color = 0x9c24e8;
    else if (type.compare("equip-ammo") == 0)
        color = 0x8b6311;
    else
        color = 0x000000;

    return color;
}

unsigned int ItemPopup::getNumRows()
{
    return mItemDesc->getNumberOfRows() + mItemEffect->getNumberOfRows() +
           mItemWeight->getNumberOfRows();
}

void ItemPopup::view(int x, int y)
{
    if (windowContainer->getWidth() < (x + getWidth() + 5))
        x = windowContainer->getWidth() - getWidth();
    if ((y - getHeight() - 10) < 0)
        y = 0;
    else
        y = y - getHeight() - 10;
    setPosition(x, y);
    setVisible(true);
    requestMoveToTop();
}
