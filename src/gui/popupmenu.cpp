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

#include "popupmenu.h"

#include <cassert>
#include <iostream>

#include <guichan/focushandler.hpp>

#include "browserbox.h"
#include "inventorywindow.h"
#include "item_amount.h"
#include "windowcontainer.h"

#include "../being.h"
#include "../floor_item.h"
#include "../item.h"
#include "../localplayer.h"
#include "../npc.h"

#include "../resources/iteminfo.h"
#include "../resources/itemdb.h"

extern std::string tradePartnerName;

PopupMenu::PopupMenu():
    Window(),
    mBeing(NULL),
    mFloorItem(NULL),
    mItem(NULL)
{
    setResizable(false);
    setTitleBarHeight(0);
    mShowTitle = false;

    mBrowserBox = new BrowserBox();
    mBrowserBox->setPosition(4, 4);
    mBrowserBox->setHighlightMode(BrowserBox::BACKGROUND);
    mBrowserBox->setOpaque(false);
    add(mBrowserBox);
    mBrowserBox->setLinkHandler(this);
}

void PopupMenu::showPopup(int x, int y, Being *being)
{
    mBeing = being;
    mBrowserBox->clearRows();

    switch (mBeing->getType())
    {
        case Being::PLAYER:
            {
                // Players can be traded with. Later also attack, follow and
                // add as buddy will be options in this menu.
                const std::string &name = mBeing->getName();
                mBrowserBox->addRow("@@trade|Trade With " + name + "@@");

                mBrowserBox->addRow("@@attack|Attack " + name + "@@");
                //mBrowserBox->addRow("@@follow|Follow " + name + "@@");
                //mBrowserBox->addRow("@@buddy|Add " + name + " to Buddy List@@");
            }
            break;

        case Being::NPC:
            // NPCs can be talked to (single option, candidate for removal
            // unless more options would be added)
            mBrowserBox->addRow("@@talk|Talk To NPC@@");
            break;

        default:
            /* Other beings aren't interesting... */
            break;
    }

    //browserBox->addRow("@@look|Look To@@");
    mBrowserBox->addRow("##3---");
    mBrowserBox->addRow("@@cancel|Cancel@@");

    showPopup(x, y);
}

void PopupMenu::showPopup(int x, int y, FloorItem *floorItem)
{
    mFloorItem = floorItem;
    mBrowserBox->clearRows();

    // Floor item can be picked up (single option, candidate for removal)
    std::string name = ItemDB::get(mFloorItem->getItemId()).getName();
    mBrowserBox->addRow("@@pickup|Pick Up " + name + "@@");

    //browserBox->addRow("@@look|Look To@@");
    mBrowserBox->addRow("##3---");
    mBrowserBox->addRow("@@cancel|Cancel@@");

    showPopup(x, y);
}

void PopupMenu::handleLink(const std::string& link)
{
    // Talk To action
    if (link == "talk" &&
        mBeing != NULL &&
        mBeing->getType() == Being::NPC &&
        current_npc == 0)
    {
        dynamic_cast<NPC*>(mBeing)->talk();
    }

    // Trade action
    else if (link == "trade" &&
             mBeing != NULL &&
             mBeing->getType() == Being::PLAYER)
    {
        player_node->trade(mBeing);
        tradePartnerName = mBeing->getName();
    }

    // Attack action
    else if (link == "attack" &&
             mBeing != NULL &&
             mBeing->getType() == Being::PLAYER)
    {
        player_node->attack(mBeing, true);
    }

    /*
    // Follow Player action
    else if (link == "follow")
    {
    }*/

    /*
    // Add Buddy action
    else if ((link == "buddy") && mBeing != NULL && mBeing->isPlayer())
    {
        if (!buddyWindow->isVisible())
            buddyWindow->setVisible(true);

        buddyWindow->addBuddy(mBeing->getName());
    }*/

    // Pick Up Floor Item action
    else if ((link == "pickup") && mFloorItem != NULL)
    {
        player_node->pickUp(mFloorItem);
    }

    // Look To action
    else if (link == "look")
    {
    }

    else if (link == "use")
    {
        assert(mItem);
        if (mItem->isEquipment())
        {
            if (mItem->isEquipped())
            {
                player_node->unequipItem(mItem);
            }
            else
            {
                player_node->equipItem(mItem);
            }
        }
        else
        {
            player_node->useItem(mItem);
        }
    }

    else if (link == "drop")
    {
        new ItemAmountWindow(AMOUNT_ITEM_DROP, inventoryWindow, mItem);
    }

    else if (link == "description")
    {
        // do nothing for now, I need to write
        // a window for the description first
    }

    // Unknown actions
    else
    {
        std::cout << link << std::endl;
    }

    setVisible(false);

    /*
     * This is need cause of a bug in guichan that leave
     * the focus on the popup menu even if is not visible.
     */
    _getFocusHandler()->focusNone();

    mBeing = NULL;
    mFloorItem = NULL;
    mItem = NULL;
}

void PopupMenu::showPopup(int x, int y, Item *item)
{
    assert(item);
    mItem = item;
    mBrowserBox->clearRows();

    if (item->isEquipment())
    {
        if (item->isEquipped())
            mBrowserBox->addRow("@@use|Unequip@@");
        else
            mBrowserBox->addRow("@@use|Equip@@");
    }
    else
        mBrowserBox->addRow("@@use|Use@@");

    mBrowserBox->addRow("@@drop|Drop@@");
    mBrowserBox->addRow("@@description|Description@@");
    mBrowserBox->addRow("##3---");
    mBrowserBox->addRow("@@cancel|Cancel@@");

    showPopup(x, y);
}

void PopupMenu::showPopup(int x, int y)
{
    setContentSize(mBrowserBox->getWidth() + 8, mBrowserBox->getHeight() + 8);
    if (windowContainer->getWidth() < (x + getWidth() + 5))
        x -= (getWidth() + 50);
    if (windowContainer->getHeight() < (y + getHeight() + 5))
        y -= (getHeight() + 50);
    setPosition(x, y);
    setVisible(true);
    requestMoveToTop();
}
