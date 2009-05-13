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

#include "gui/chat.h"
#include "gui/inventorywindow.h"
#include "gui/storagewindow.h"
#include "gui/itemamount.h"

#include "gui/widgets/browserbox.h"

#include "being.h"
#include "beingmanager.h"
#include "flooritem.h"
#include "graphics.h"
#include "item.h"
#include "localplayer.h"
#include "npc.h"
#include "playerrelations.h"

#include "net/adminhandler.h"
#include "net/inventoryhandler.h"
#include "net/net.h"
#include "net/partyhandler.h"
#include "net/tradehandler.h"

#include "resources/itemdb.h"
#include "resources/iteminfo.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <cassert>

extern std::string tradePartnerName;

PopupMenu::PopupMenu():
    Popup("PopupMenu"),
    mBeingId(0),
    mFloorItem(NULL),
    mItem(NULL)
{
    mBrowserBox = new BrowserBox;
    mBrowserBox->setPosition(4, 4);
    mBrowserBox->setHighlightMode(BrowserBox::BACKGROUND);
    mBrowserBox->setOpaque(false);
    mBrowserBox->setLinkHandler(this);
    add(mBrowserBox);
}

void PopupMenu::showPopup(int x, int y, Being *being)
{
    mBeingId = being->getId();
    mBrowserBox->clearRows();

    // Any being's name can be added to chat
    if (being->getType() != Being::UNKNOWN)
        mBrowserBox->addRow(_("@@name|Add name to chat@@"));

    const std::string &name = being->getType() == Being::NPC ?
                              being->getName().substr(0, being->getName().size()
                              - 6) : being->getName();

    switch (being->getType())
    {
        case Being::PLAYER:
            {
                // Players can be traded with. Later also follow and
                // add as buddy will be options in this menu.
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

                //mBrowserBox->addRow(_(strprintf("@@follow|Follow %s@@"), name.c_str()));
                //mBrowserBox->addRow(_("@@buddy|Add ") + name + " to Buddy List@@");
#ifdef TMWSERV_SUPPORT
                mBrowserBox->addRow(strprintf(_("@@guild|Invite %s to join your guild@@"), name.c_str()));
#endif
                if (player_node->isInParty())
                    mBrowserBox->addRow(strprintf(_("@@party|Invite %s to join your party@@"), name.c_str()));

                if (player_node->isGM())
                {
                    mBrowserBox->addRow("##3---");
                    mBrowserBox->addRow(_("@@admin-kick|Kick player@@"));
                }
            }
            break;

        case Being::NPC:
            // NPCs can be talked to (single option, candidate for removal
            // unless more options would be added)
            mBrowserBox->addRow(strprintf(_("@@talk|Talk To %s@@"), name.c_str()));
            break;

        case Being::MONSTER:
            {
                // Monsters can be attacked
                mBrowserBox->addRow(strprintf(_("@@attack|Attack %s@@"), name.c_str()));

                if (player_node->isGM())
                    mBrowserBox->addRow(_("@@admin-kick|Kick monster@@"));
            }
            break;

        default:
            /* Other beings aren't interesting... */
            return;
    }

    //browserBox->addRow("@@look|Look To@@");
    mBrowserBox->addRow("##3---");
    mBrowserBox->addRow(_("@@cancel|Cancel@@"));

    showPopup(x, y);
}

void PopupMenu::showPopup(int x, int y, FloorItem *floorItem)
{
    mFloorItem = floorItem;
    mItem = floorItem->getItem();
    mBrowserBox->clearRows();

    // Floor item can be picked up (single option, candidate for removal)
    std::string name = ItemDB::get(mFloorItem->getItemId()).getName();
    mBrowserBox->addRow(strprintf(_("@@pickup|Pick up %s@@"), name.c_str()));
    mBrowserBox->addRow(_("@@chat|Add to chat@@"));

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
        Net::getTradeHandler()->request(being);
        tradePartnerName = being->getName();
    }
#ifdef EATHENA_SUPPORT
    // Attack action
    else if (link == "attack" && being)
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
            if (mItem->isEquipped())
                Net::getInventoryHandler()->unequipItem(mItem);
            else
                Net::getInventoryHandler()->equipItem(mItem);
        }
        else
        {
            Net::getInventoryHandler()->useItem(mItem);
        }
    }

    else if (link == "chat")
    {
        chatWindow->addItemText(mItem->getInfo().getName());
    }

    else if (link == "split")
    {
        ItemAmountWindow::showWindow(ItemAmountWindow::ItemSplit,
                             inventoryWindow, mItem);
    }

    else if (link == "drop")
    {
        ItemAmountWindow::showWindow(ItemAmountWindow::ItemDrop,
                             inventoryWindow, mItem);
    }

    else if (link == "store")
    {
        ItemAmountWindow::showWindow(ItemAmountWindow::StoreAdd,
                             inventoryWindow, mItem);
    }

    else if (link == "retrieve")
    {
        ItemAmountWindow::showWindow(ItemAmountWindow::StoreRemove,
                             storageWindow, mItem);
    }

    else if (link == "party" && being && being->getType() == Being::PLAYER)
    {
        Net::getPartyHandler()->invite(dynamic_cast<Player*>(being));
    }

    else if (link == "name" && being)
    {
        const std::string &name = being->getType() == Being::NPC ?
                                  being->getName().substr(0,
                                  being->getName().size() - 6) : being->getName();
        chatWindow->addInputText(name);
    }

    else if (link == "admin-kick" &&
             being &&
             (being->getType() == Being::PLAYER ||
              being->getType() == Being::MONSTER))
    {
        Net::getAdminHandler()->kick(being->getId());
    }

    // Unknown actions
    else if (link != "cancel")
    {
        std::cout << link << std::endl;
    }

    setVisible(false);

    mBeingId = 0;
    mFloorItem = NULL;
    mItem = NULL;
}

void PopupMenu::showPopup(int x, int y, Item *item, bool isInventory)
{
    assert(item);
    mItem = item;
    mBrowserBox->clearRows();

    if (isInventory)
    {
        if (item->isEquipment())
        {
            if (item->isEquipped())
                mBrowserBox->addRow(_("@@use|Unequip@@"));
            else
                mBrowserBox->addRow(_("@@use|Equip@@"));
        }
        else
            mBrowserBox->addRow(_("@@use|Use@@"));

        mBrowserBox->addRow(_("@@drop|Drop@@"));

        if (Net::getInventoryHandler()->canSplit(item))
        {
            mBrowserBox->addRow(_("@@split|Split@@"));
        }

        if (player_node->getInStorage())
        {
            mBrowserBox->addRow(_("@@store|Store@@"));
        }
    }
    // Assume in storage for now
    // TODO: make this whole system more flexible, if needed
    else
    {
        mBrowserBox->addRow(_("@@retrieve|Retrieve@@"));
    }
    mBrowserBox->addRow(_("@@chat|Add to chat@@"));
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
