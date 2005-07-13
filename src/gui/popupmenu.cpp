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
#include "../graphics.h"
#include "../game.h"
#include "../engine.h"
#include "../net/network.h"
#include "../resources/itemmanager.h"
#include <iostream>

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
    mX = -1;
    mY = -1;
}

PopupMenu::~PopupMenu()
{
    delete browserBox;
    delete floorItem;
}

void PopupMenu::setVisible(bool visible)
{
    if (visible == false)
    {
        if (hasFocus())
        {
            mFocusHandler->focusNone();
        }
        setPosition(screen->w, screen->h);
    }

    mVisible = visible;
}

void PopupMenu::showPopup(int mx, int my)
{
    being = findNode(mx, my);
    floorItem = find_floor_item_by_id(find_floor_item_by_cor(mx, my));
    mX = mx;
    mY = my;
    browserBox->clearRows();

    if (being && being->isNpc())
    {
        // NPCs can be talked to (single option, candidate for removal unless
        // more options would be added)
        browserBox->addRow("@@talk|Talk To NPC@@");
    }
    else if (being && being->isPlayer())
    {
        // Players can be traded with. Later also attack, follow and add as
        // buddy will be options in this menu.

        std::string name = being->name;
        //browserBox->addRow("@@attack|Attack " + name + "@@");
        browserBox->addRow("@@trade|Trade With " + name + "@@");
        //browserBox->addRow("@@follow|Follow " + name + "@@");
        //browserBox->addRow("@@buddy|Add " + name + " to Buddy List@@");
    }
    else if (floorItem)
    {
        // Floor item can be picked up (single option, candidate for removal)
        std::string name = itemDb->getItemInfo(floorItem->id)->getName();
        browserBox->addRow("@@pickup|Pick Up " + name + "@@");
    }
    else
    {
        // If there is nothing of interest, don't display menu.
        return;
    }
   
    //browserBox->addRow("@@look|Look To@@");
    browserBox->addRow("##3---");
    browserBox->addRow("@@cancel|Cancel@@");

    setContentSize(browserBox->getWidth() + 8, browserBox->getHeight() + 8);
    mx = (mx - camera_x) * 32 + 25;
    my = (my - camera_y) * 32 + 25;
    if (screen->w < (mx + getWidth() + 5))
        mx -= (getWidth() + 50);
    if (screen->h < (my + getHeight() + 5))
        my -= (getHeight() + 50);
    setPosition(mx, my);
    setVisible(true);
}

void PopupMenu::handleLink(const std::string& link)
{
    // Talk To action
    if ((link == "talk") && being && being->isNpc() &&
            (current_npc == 0))
    {
        WFIFOW(0) = net_w_value(0x0090);
        WFIFOL(2) = net_l_value(being->id);
        WFIFOB(6) = 0;
        WFIFOSET(7);
        current_npc = being->id;
    }

    // Trade action
    else if ((link == "trade") && being && being->isPlayer())
    {
        WFIFOW(0) = net_w_value(0x00e4);
        WFIFOL(2) = net_l_value(being->id);
        WFIFOSET(6);
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

    // Unknown actions
    else
    {
        std::cout << link << std::endl;
    }

    setVisible(false);

    being = NULL;
    floorItem = NULL;
    mX = -1;
    mY = -1;
}
