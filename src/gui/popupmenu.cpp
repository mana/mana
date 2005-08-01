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

#include <assert.h>

#include "popupmenu.h"
#include "gui.h"
#include "inventorywindow.h"
#include "item_amount.h"
#include "../graphics.h"
#include "../game.h"
#include "../net/network.h"
#include "../resources/itemmanager.h"
#include "../item.h"
#include "../inventory.h"


PopupMenu::PopupMenu():
    Window()
{
    setResizable(false);
    setTitleBarHeight(0);
    title = false;

    browserBox = new BrowserBox();
    browserBox->setPosition(4, 4);
    browserBox->setHighlightMode(BrowserBox::BACKGROUND);
    browserBox->setOpaque(false);
    add(browserBox);
    browserBox->setLinkHandler(this);

    being = NULL;
    floorItem = NULL;
}

PopupMenu::~PopupMenu()
{
    delete browserBox;
    delete floorItem;
}

void PopupMenu::showPopup(int x, int y, Being *being)
{
    std::string name;

    this->being = being;
    browserBox->clearRows();

    switch (being->getType())
    {
        case Being::PLAYER:
            // Players can be traded with. Later also attack, follow and
            // add as buddy will be options in this menu.

            name = being->name;
            //browserBox->addRow("@@attack|Attack " + name + "@@");
            browserBox->addRow("@@trade|Trade With " + name + "@@");
            //browserBox->addRow("@@follow|Follow " + name + "@@");
            //browserBox->addRow("@@buddy|Add " + name + " to Buddy List@@");
            break;

        case Being::NPC:
            // NPCs can be talked to (single option, candidate for removal
            // unless more options would be added)
            browserBox->addRow("@@talk|Talk To NPC@@");
            break;

        default:
            /* Other beings aren't interesting... */
            break;
    }

    //browserBox->addRow("@@look|Look To@@");
    browserBox->addRow("##3---");
    browserBox->addRow("@@cancel|Cancel@@");

    showPopup(x, y);
}

void PopupMenu::showPopup(int x, int y, FloorItem *floorItem)
{
    this->floorItem = floorItem;
    browserBox->clearRows();

    // Floor item can be picked up (single option, candidate for removal)
    std::string name = itemDb->getItemInfo(floorItem->id)->getName();
    browserBox->addRow("@@pickup|Pick Up " + name + "@@");

    //browserBox->addRow("@@look|Look To@@");
    browserBox->addRow("##3---");
    browserBox->addRow("@@cancel|Cancel@@");

    showPopup(x, y);
}

void PopupMenu::handleLink(const std::string& link)
{
    // Talk To action
    if ((link == "talk") && being && being->getType() == Being::NPC &&
            (current_npc == 0))
    {
        WFIFOW(0) = net_w_value(0x0090);
        WFIFOL(2) = net_l_value(being->getId());
        WFIFOB(6) = 0;
        WFIFOSET(7);
        current_npc = being->getId();
    }

    // Trade action
    else if ((link == "trade") && being && being->getType() == Being::PLAYER)
    {
        WFIFOW(0) = net_w_value(0x00e4);
        WFIFOL(2) = net_l_value(being->getId());
        WFIFOSET(6);
        //tradePartner.flush();
        //tradePartner << "Trade: You and " << being->name<< "";
        strcpy(tradePartnerName, being->name);
    }
    /*
    // Follow Player action
    else if (link == "follow")
    {
    }*/

    /*
    // Add Buddy action
    else if ((link == "buddy") && being && being->isPlayer())
    {
        if (!buddyWindow->isVisible())
            buddyWindow->setVisible(true);

        buddyWindow->addBuddy(being->name);
    }*/

    // Pick Up Floor Item action
    else if ((link == "pickup") && floorItem)
    {
        WFIFOW(0) = net_w_value(0x009f);
        WFIFOL(2) = net_l_value(floorItem->int_id);
        WFIFOSET(6);
    }

    // Look To action
    else if (link == "look")
    {
    }

    else if (link == "use")
    {
        assert(m_item);
        if (m_item->isEquipment())
        {
            if (m_item->isEquipped())
            {
                inventory->unequipItem(m_item);
            }
            else
            {
                inventory->equipItem(m_item);
            }
        }
        else
        {
            inventory->useItem(m_item);
        }
    }

    else if (link == "drop")
    {
                new ItemAmountWindow(AMOUNT_ITEM_DROP, inventoryWindow);
    }

    else if (link == "description")
    {
        // do nothing for now, I need to write
        // a window for the description first
        ;
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

    being = NULL;
    floorItem = NULL;
    m_item = NULL;
}

void PopupMenu::showPopup(int x, int y, Item *item)
{
    assert(item);
    m_item = item;
    browserBox->clearRows();

    if (item->isEquipment())
    {
        if (item->isEquipped())
            browserBox->addRow("@@use|Unequip@@");
        else
            browserBox->addRow("@@use|Equip@@");
    }
    else
        browserBox->addRow("@@use|Use@@");

    browserBox->addRow("@@drop|Drop@@");
    browserBox->addRow("@@description|Description@@");
    browserBox->addRow("##3---");
    browserBox->addRow("@@cancel|Cancel@@");

    showPopup(x, y);
}

void PopupMenu::showPopup(int x, int y)
{
    setContentSize(browserBox->getWidth() + 8, browserBox->getHeight() + 8);
    if (guiGraphics->getWidth() < (x + getWidth() + 5))
        x -= (getWidth() + 50);
    if (guiGraphics->getHeight() < (y + getHeight() + 5))
        y -= (getHeight() + 50);
    setPosition(x, y);
    setVisible(true);
    requestMoveToTop();
}
