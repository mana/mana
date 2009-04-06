/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#include "gui/popupmenu.h"

#include "gui/browserbox.h"
#include "gui/chat.h"
#include "gui/inventorywindow.h"
#include "gui/item_amount.h"

#include "being.h"
#include "beingmanager.h"
#include "floor_item.h"
#include "graphics.h"
#include "item.h"
#include "localplayer.h"
#include "npc.h"
#include "player_relations.h"

#include "net/net.h"
#ifdef EATHENA_SUPPORT
#include "net/ea/adminhandler.h"
#endif

#include "resources/itemdb.h"
#include "resources/iteminfo.h"

#include "utils/gettext.h"
#include "utils/strprintf.h"

#include <cassert>

extern std::string tradePartnerName;

PopupMenu::PopupMenu():
    Window("PopupMenu"),
    mBeingId(0),
    mFloorItem(NULL),
    mItem(NULL)
{
    setResizable(false);
    setTitleBarHeight(0);
    setShowTitle(false);

    mBrowserBox = new BrowserBox;
    mBrowserBox->setPosition(4, 4);
    mBrowserBox->setHighlightMode(BrowserBox::BACKGROUND);
    mBrowserBox->setOpaque(false);
    mBrowserBox->setLinkHandler(this);
    add(mBrowserBox);

    loadWindowState();
}

void PopupMenu::showPopup(int x, int y, Being *being)
{
    mBeingId = being->getId();
    mBrowserBox->clearRows();

    switch (being->getType())
    {
        case Being::PLAYER:
            {
                // Players can be traded with. Later also follow and
                // add as buddy will be options in this menu.
                const std::string &name = being->getName();
                mBrowserBox->addRow(strprintf(_("@@trade|Trade With %s@@"), name.c_str()));
                mBrowserBox->addRow(strprintf(_("@@attack|Attack %s@@"), name.c_str()));

                mBrowserBox->addRow("##3---");

                switch (player_relations.getRelation(name)) {
                    case PlayerRelation::NEUTRAL:
                        mBrowserBox->addRow(strprintf(_("@@friend|Befriend %s@@"), name.c_str()));

                    case PlayerRelation::FRIEND:
                        mBrowserBox->addRow(strprintf(_("@@disregard|Disregard %s@@"), name.c_str()));
                        mBrowserBox->addRow(strprintf(_("@@ignore|Ignore %s@@"), name.c_str()));
                        break;

                    case PlayerRelation::DISREGARDED:
                        mBrowserBox->addRow(strprintf(_("@@unignore|Un-Ignore %s@@"), name.c_str()));
                        mBrowserBox->addRow(strprintf(_("@@ignore|Completely ignore %s@@"), name.c_str()));
                        break;

                    case PlayerRelation::IGNORED:
                        mBrowserBox->addRow(strprintf(_("@@unignore|Un-Ignore %s@@"), name.c_str()));
                        break;
                }

                //mBrowserBox->addRow(_("@@follow|Follow ") + name + "@@");
                //mBrowserBox->addRow(_("@@buddy|Add ") + name + " to Buddy List@@");
                mBrowserBox->addRow(strprintf(_("@@guild|Invite %s@@"), name.c_str()));
                mBrowserBox->addRow(strprintf(_("@@party|Invite %s to join your party@@"), name.c_str()));

                mBrowserBox->addRow("##3---");
                mBrowserBox->addRow(strprintf(_("@@party-invite|Invite %s to party@@"), name.c_str()));
                //mBrowserBox->addRow(_("@@admin-kick|Kick player@@"));
            }
            break;

        case Being::NPC:
            // NPCs can be talked to (single option, candidate for removal
            // unless more options would be added)
            mBrowserBox->addRow(_("@@talk|Talk To NPC@@"));
            break;

        /*case Being::MONSTER:
            mBrowserBox->addRow(_("@@admin-kick|Kick monster@@"));
            break;*/

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

void PopupMenu::handleLink(const std::string &link)
{
    Being *being = beingManager->findBeing(mBeingId);

    // Talk To action
    if (link == "talk" &&
        being &&
        being->getType() == Being::NPC &&
        current_npc == 0)
    {
        dynamic_cast<NPC*>(being)->talk();
    }

    // Trade action
    else if (link == "trade" &&
             being &&
             being->getType() == Being::PLAYER)
    {
        player_node->trade(being);
        tradePartnerName = being->getName();
    }
#ifdef EATHENA_SUPPORT
    // Attack action
    else if (link == "attack" &&
             being &&
             being->getType() == Being::PLAYER)
    {
        player_node->attack(being, true);
    }
#endif
    else if (link == "unignore" &&
             being &&
             being->getType() == Being::PLAYER)
    {
        player_relations.setRelation(being->getName(), PlayerRelation::NEUTRAL);
    }

    else if (link == "ignore" &&
             being &&
             being->getType() == Being::PLAYER)
    {
        player_relations.setRelation(being->getName(), PlayerRelation::IGNORED);
    }

    else if (link == "disregard" &&
             being &&
             being->getType() == Being::PLAYER)
    {
        player_relations.setRelation(being->getName(), PlayerRelation::DISREGARDED);
    }

    else if (link == "friend" &&
             being &&
             being->getType() == Being::PLAYER)
    {
        player_relations.setRelation(being->getName(), PlayerRelation::FRIEND);
    }
#ifdef TMWSERV_SUPPORT
    // Guild action
    else if (link == "guild" &&
             being != NULL &&
             being->getType() == Being::PLAYER)
    {
        player_node->inviteToGuild(being);
    }
#endif
    // Add player to your party
    else if (link == "party")
    {
        player_node->inviteToParty(dynamic_cast<Player*>(being));
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

        buddyWindow->addBuddy(being->getName());
    }*/

    // Pick Up Floor Item action
    else if ((link == "pickup") && mFloorItem)
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
#ifdef TMWSERV_SUPPORT
            player_node->equipItem(mItem);
#else
            if (mItem->isEquipped())
            {
                player_node->unequipItem(mItem);
            }
            else
            {
                player_node->equipItem(mItem);
            }
#endif
        }
        else
        {
            player_node->useItem(mItem);
        }
    }

    else if (link == "chat")
    {
        chatWindow->addItemText(mItem->getInfo().getName());
    }

    else if (link == "split")
    {
        new ItemAmountWindow(ItemAmountWindow::ItemSplit,
                             inventoryWindow, mItem);
    }
    else if (link == "drop")
    {
        new ItemAmountWindow(ItemAmountWindow::ItemDrop,
                             inventoryWindow, mItem);
    }
#ifdef EATHENA_SUPPORT
    else if (link == "party-invite" &&
             being &&
             being->getType() == Being::PLAYER)
    {
        player_node->inviteToParty(dynamic_cast<Player*> (being));
    }
    else if (link == "admin-kick" &&
             being &&
             (being->getType() == Being::PLAYER ||
              being->getType() == Being::MONSTER))
    {
        Net::getAdminHandler()->kick(being->getId());
    }
#endif

    // Unknown actions
    else
    {
        std::cout << link << std::endl;
    }

    setVisible(false);

    mBeingId = 0;
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
#ifdef TMWSERV_SUPPORT
        mBrowserBox->addRow(_("@@use|Equip@@"));
#else
        if (item->isEquipped())
            mBrowserBox->addRow(_("@@use|Unequip@@"));
        else
            mBrowserBox->addRow(_("@@use|Equip@@"));
#endif
    }
    else
        mBrowserBox->addRow(_("@@use|Use@@"));

    mBrowserBox->addRow(_("@@drop|Drop@@"));
#ifdef TMWSERV_SUPPORT
    if (!item->isEquipment())
        mBrowserBox->addRow(_("@@split|Split@@"));
#endif
    mBrowserBox->addRow(_("@@chat|Add to Chat@@"));
    mBrowserBox->addRow("##3---");
    mBrowserBox->addRow(_("@@cancel|Cancel@@"));

    showPopup(x, y);
}

void PopupMenu::showPopup(int x, int y)
{
    setContentSize(mBrowserBox->getWidth() + 8, mBrowserBox->getHeight() + 8);
    if (graphics->getWidth() < (x + getWidth() + 5))
        x = graphics->getWidth() - getWidth();
    if (graphics->getHeight() < (y + getHeight() + 5))
        y = graphics->getHeight() - getHeight();
    setPosition(x, y);
    setVisible(true);
    requestMoveToTop();
}
