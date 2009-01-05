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

#include "../utils/gettext.h"
#include "../utils/strprintf.h"

extern std::string tradePartnerName;

PopupMenu::PopupMenu():
    Window(),
    mBeing(NULL),
    mFloorItem(NULL),
    mItem(NULL)
{
    setResizable(false);
    setTitleBarHeight(0);

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
                mBrowserBox->addRow(strprintf(_("@@trade|Trade With %s@@"), name.c_str()));
                mBrowserBox->addRow(strprintf(_("@@attack|Attack %s@@"), name.c_str()));
                //mBrowserBox->addRow("@@follow|Follow " + name + "@@");
                //mBrowserBox->addRow("@@buddy|Add " + name + " to Buddy List@@");
                mBrowserBox->addRow(strprintf(_("@@guild|Invite %s@@"), name.c_str()));
                mBrowserBox->addRow(strprintf(_("@@party|Invite %s to join your party@@"), name.c_str()));
            }
            break;

        case Being::NPC:
            // NPCs can be talked to (single option, candidate for removal
            // unless more options would be added)
            mBrowserBox->addRow(_("@@talk|Talk To NPC@@"));
            break;

        default:
            /* Other beings aren't interesting... */
            break;
    }

    //browserBox->addRow("@@look|Look To@@");
    mBrowserBox->addRow("##3---");
    mBrowserBox->addRow(_("@@cancel|Cancel@@"));

    showPopup(x, y);
}

void PopupMenu::showPopup(int x, int y, FloorItem *floorItem)
{
    mFloorItem = floorItem;
    mBrowserBox->clearRows();

    // Floor item can be picked up (single option, candidate for removal)
    std::string name = ItemDB::get(mFloorItem->getItemId()).getName();
    mBrowserBox->addRow(strprintf(_("@@pickup|Pick Up %s@@"), name.c_str()));

    //browserBox->addRow("@@look|Look To@@");
    mBrowserBox->addRow("##3---");
    mBrowserBox->addRow(_("@@cancel|Cancel@@"));

    showPopup(x, y);
}

void PopupMenu::handleLink(const std::string& link)
{
    // Talk To action
    if (link == "talk" &&
        mBeing != NULL &&
        mBeing->getType() == Being::NPC)
    {
        static_cast<NPC*>(mBeing)->talk();
    }

    // Trade action
    else if (link == "trade" &&
             mBeing != NULL &&
             mBeing->getType() == Being::PLAYER)
    {
        player_node->trade(mBeing);
        tradePartnerName = mBeing->getName();
    }

    // Guild action
    else if (link == "guild" &&
             mBeing != NULL &&
             mBeing->getType() == Being::PLAYER)
    {
        player_node->inviteToGuild(mBeing);
    }

    // Add player to your party
    else if (link == "party")
    {
        player_node->inviteToParty(mBeing->getName());
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
            player_node->equipItem(mItem);
        }
        else
        {
            player_node->useItem(mItem->getInvIndex());
        }
    }
    else if (link == "split")
    {
        new ItemAmountWindow(AMOUNT_ITEM_SPLIT, inventoryWindow, mItem);
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
        mBrowserBox->addRow(_("@@use|Equip@@"));
    }
    else
        mBrowserBox->addRow(_("@@use|Use@@"));
    mBrowserBox->addRow(_("@@drop|Drop@@"));
    mBrowserBox->addRow(_("@@description|Description@@"));
    if (!item->isEquipment())
    { mBrowserBox->addRow(_("@@split|Split@@")); }
    mBrowserBox->addRow("##3---");
    mBrowserBox->addRow(_("@@cancel|Cancel@@"));

    showPopup(x, y);
}

void PopupMenu::showPopup(int x, int y)
{
    setContentSize(mBrowserBox->getWidth() + 8, mBrowserBox->getHeight() + 8);
    if (windowContainer->getWidth() < (x + getWidth() + 5))
        x = windowContainer->getWidth() - getWidth();
    if (windowContainer->getHeight() < (y + getHeight() + 5))
        y = windowContainer->getHeight() - getHeight();
    setPosition(x, y);
    setVisible(true);
    requestMoveToTop();
}
