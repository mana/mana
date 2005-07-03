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
#include "../engine.h"
#include "../game.h"
#include <iostream>

PopupMenu::PopupMenu():
    Window()
{
    setResizable(false);
    setTitleBarHeight(0);

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
    delete being;
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

    if (being)
    {
        if (being->isMonster())
        {
            browserBox->addRow("@@attack|Attack Monster@@");
        }
        else if (being->isNpc())
        {
            browserBox->addRow("@@talk|Talk To NPC@@");
        }
        else if (being->isPlayer())
        {
            std::string name = being->name;
            //browserBox->addRow("@@attack|Attack " + name + "@@");
            browserBox->addRow("@@trade|Trade With " + name + "@@");
            browserBox->addRow("@@follow|Follow " + name + "@@");
            //browserBox->addRow("@@buddy|Add " + name + " to Buddy List@@");
        }
    }
    else if (floorItem)
    {
        std::string name = itemDb->getItemInfo(floorItem->id)->getName();
        browserBox->addRow("@@pickup|Pick Up " + name + "@@");
    }
    else
    {
        browserBox->addRow("@@walk|Walk To@@");
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

void PopupMenu::draw(gcn::Graphics* graphics)
{
    int x, y;
    getAbsolutePosition(x, y);

    ((Graphics*) graphics)->drawImageRect(x, y, getWidth(), getHeight(),
                                          border);

    if (mContent != NULL)
    {
        graphics->pushClipArea(getContentDimension());
        graphics->pushClipArea(gcn::Rectangle(
                    0, 0, mContent->getWidth(), mContent->getHeight()));
        mContent->draw(graphics);
        graphics->popClipArea();
        graphics->popClipArea();
    }
}

void PopupMenu::handleLink(const std::string& link)
{
    // Attack action
    if ((link == "attack") && being)
    {
        if (being->isMonster() && (being->action != MONSTER_DEAD))
        {
            autoTarget = being;
            attack(being);
        }
    }

    // Talk To action
    else if ((link == "talk") && being && being->isNpc() &&
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

    // Follow Player action
    else if (link == "follow")
    {
    }
    
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

    // Walk To action
    else if ((link == "walk") && (mX != -1) && (mY != -1))
    {
        walk(mX, mY, 0);
        player_node->setDestination(mX, mY);
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
