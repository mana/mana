/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "gui/popupmenu.h"

#include "actorspritemanager.h"
#include "being.h"
#include "flooritem.h"
#include "graphics.h"
#include "item.h"
#include "localplayer.h"
#include "log.h"
#include "playerinfo.h"
#include "playerrelations.h"

#include "gui/chat.h"
#include "gui/equipmentwindow.h"
#include "gui/inventorywindow.h"
#include "gui/itemamount.h"

#include "gui/widgets/browserbox.h"

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

std::string tradePartnerName;

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

    const std::string &name = being->getName();

    switch (being->getType())
    {
        case ActorSprite::PLAYER:
            {
                // Players can be traded with.
                mBrowserBox->addRow(strprintf("@@trade|%s@@",
                                                strprintf(_("Trade with %s..."),
                                                    name.c_str()).c_str()));
                // TRANSLATORS: Attacking a player.
                mBrowserBox->addRow(strprintf("@@attack|%s@@",
                                                strprintf(_("Attack %s"),
                                                    name.c_str()).c_str()));
                // TRANSLATORS: Whispering a player.
                mBrowserBox->addRow(strprintf("@@whisper|%s@@",
                                                strprintf(_("Whisper %s"),
                                                    name.c_str()).c_str()));

                mBrowserBox->addRow("##3---");

                switch (player_relations.getRelation(name))
                {
                    case PlayerRelation::NEUTRAL:
                        mBrowserBox->addRow(strprintf("@@friend|%s@@",
                                                strprintf(_("Befriend %s"),
                                                    name.c_str()).c_str()));

                    case PlayerRelation::FRIEND:
                        mBrowserBox->addRow(strprintf("@@disregard|%s@@",
                                                strprintf(_("Disregard %s"),
                                                    name.c_str()).c_str()));
                        mBrowserBox->addRow(strprintf("@@ignore|%s@@",
                                                strprintf(_("Ignore %s"),
                                                    name.c_str()).c_str()));
                        break;

                    case PlayerRelation::DISREGARDED:
                        mBrowserBox->addRow(strprintf("@@unignore|%s@@",
                                                strprintf(_("Unignore %s"),
                                                    name.c_str()).c_str()));
                        mBrowserBox->addRow(strprintf("@@ignore|%s@@",
                                           strprintf(_("Completely ignore %s"),
                                                    name.c_str()).c_str()));
                        break;

                    case PlayerRelation::IGNORED:
                        mBrowserBox->addRow(strprintf("@@unignore|%s@@",
                                                strprintf(_("Unignore %s"),
                                                    name.c_str()).c_str()));
                        break;
                }

                if (player_node->getNumberOfGuilds())
                    mBrowserBox->addRow(strprintf("@@guild|%s@@",
                                strprintf(_("Invite %s to join your guild"),
                                                    name.c_str()).c_str()));
                if (player_node->isInParty() ||
                    Net::getNetworkType() == ServerInfo::MANASERV)
                {
                    mBrowserBox->addRow(strprintf("@@party|%s@@",
                                strprintf(_("Invite %s to join your party"),
                                                    name.c_str()).c_str()));
                }

                if (player_node->isGM())
                {
                    mBrowserBox->addRow("##3---");
                    mBrowserBox->addRow(strprintf("@@admin-kick|%s@@",
                                                  _("Kick player")));
                }
            }
            break;

        case ActorSprite::NPC:
            // NPCs can be talked to (single option, candidate for removal
            // unless more options would be added)
            mBrowserBox->addRow(strprintf("@@talk|%s@@",
                                              strprintf(_("Talk to %s"),
                                                    name.c_str()).c_str()));
            break;

        case ActorSprite::MONSTER:
            {
                // Monsters can be attacked
                mBrowserBox->addRow(strprintf("@@attack|%s@@",
                                              strprintf(_("Attack %s"),
                                                    name.c_str()).c_str()));

                if (player_node->isGM())
                    mBrowserBox->addRow(strprintf("@@admin-kick|%s@@",
                                                  _("Kick monster")));
            }
            break;

        default:
            /* Other beings aren't interesting... */
            return;
    }
    mBrowserBox->addRow(strprintf("@@name|%s@@", _("Add name to chat")));

    //mBrowserBox->addRow(strprintf("@@look|%s@@", _("Look To")));
    mBrowserBox->addRow("##3---");
    mBrowserBox->addRow(strprintf("@@cancel|%s@@", _("Cancel")));

    showPopup(x, y);
}

void PopupMenu::showPopup(int x, int y, FloorItem *floorItem)
{
    mFloorItem = floorItem;
    ItemInfo info = floorItem->getInfo();
    mBrowserBox->clearRows();

    // Floor item can be picked up (single option, candidate for removal)
    std::string name = info.getName();
    mBrowserBox->addRow(strprintf("@@pickup|%s@@", strprintf(_("Pick up %s"),
                                                    name.c_str()).c_str()));
    mBrowserBox->addRow(strprintf("@@chat|%s@@", _("Add to chat")));

    //mBrowserBox->addRow(strprintf("@@look|%s@@", _("Look To")));
    mBrowserBox->addRow("##3---");
    mBrowserBox->addRow(strprintf("@@cancel|%s@@", _("Cancel")));

    showPopup(x, y);
}

void PopupMenu::handleLink(const std::string &link)
{
    Being *being = actorSpriteManager->findBeing(mBeingId);

    // Talk To action
    if (link == "talk" && being && being->canTalk())
    {
        being->talkTo();
    }

    // Trade action
    else if (link == "trade" && being &&
             being->getType() == ActorSprite::PLAYER)
    {
        Net::getTradeHandler()->request(being);
        tradePartnerName = being->getName();
    }
    // Attack action
    else if (link == "attack" && being)
    {
        player_node->attack(being, true);
    }
    else if (link == "whisper" && being)
    {
        chatWindow->addInputText("/w \"" + being->getName() + "\" ");
    }
    else if (link == "unignore" && being &&
             being->getType() == ActorSprite::PLAYER)
    {
        player_relations.setRelation(being->getName(), PlayerRelation::NEUTRAL);
    }

    else if (link == "ignore" && being &&
             being->getType() == ActorSprite::PLAYER)
    {
        player_relations.setRelation(being->getName(), PlayerRelation::IGNORED);
    }

    else if (link == "disregard" && being &&
             being->getType() == ActorSprite::PLAYER)
    {
        player_relations.setRelation(being->getName(), PlayerRelation::DISREGARDED);
    }

    else if (link == "friend" && being &&
             being->getType() == ActorSprite::PLAYER)
    {
        player_relations.setRelation(being->getName(), PlayerRelation::FRIEND);
    }
    // Guild action
    else if (link == "guild" && being &&
             being->getType() == ActorSprite::PLAYER)
    {
        player_node->inviteToGuild(being);
    }

    // Pick Up Floor Item action
    else if ((link == "pickup") && mFloorItem)
    {
        player_node->pickUp(mFloorItem);
    }

    // Look To action
    else if (link == "look")
    {
    }

    else if (link == "activate" || link == "equip" || link == "unequip")
    {
        assert(mItem);
        if (mItem->isEquippable())
        {
            if (mItem->isEquipped())
            {
                PlayerInfo::getEquipment()->triggerUnequip(
                                                equipmentWindow->getSelected());
            }
            else
            {
                mItem->doEvent(Event::DoEquip);
            }
        }
        else
        {
            mItem->doEvent(Event::DoUse);
        }
    }
    else if (link == "chat")
    {
        if (mItem)
            chatWindow->addItemText(mItem->getInfo().getName());
        else if (mFloorItem)
            chatWindow->addItemText(mFloorItem->getInfo().getName());
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
        ItemAmountWindow::showWindow(ItemAmountWindow::StoreRemove, mWindow,
                                     mItem);
    }

    else if (link == "party" && being &&
             being->getType() == ActorSprite::PLAYER)
    {
        Net::getPartyHandler()->invite(being);
    }

    else if (link == "name" && being)
    {
        const std::string &name = being->getName();
        chatWindow->addInputText(name);
    }

    else if (link == "admin-kick" &&
             being &&
             (being->getType() == ActorSprite::PLAYER ||
              being->getType() == ActorSprite::MONSTER))
    {
        Net::getAdminHandler()->kick(being->getId());
    }

    // Unknown actions
    else if (link != "cancel")
    {
        logger->log("PopupMenu: Warning, unknown action '%s'", link.c_str());
    }

    setVisible(false);

    mBeingId = 0;
    mFloorItem = NULL;
    mItem = NULL;
}

void PopupMenu::showPopup(Window *parent, int x, int y, Item *item,
                          bool isInventory, bool canDrop)
{
    assert(item);
    mItem = item;
    mWindow = parent;
    mBrowserBox->clearRows();

    if (isInventory)
    {
        if (PlayerInfo::getStorageCount() > 0)
        {
            mBrowserBox->addRow(strprintf("@@store|%s@@", _("Store")));
        }

        if (item->getInfo().getEquippable())
        {
            if (item->isEquipped())
                mBrowserBox->addRow(strprintf("@@unequip|%s@@", _("Unequip")));
            else
                mBrowserBox->addRow(strprintf("@@equip|%s@@", _("Equip")));
        }
        if (item->getInfo().getActivatable())
            mBrowserBox->addRow(strprintf("@@activate|%s@@", _("Activate")));

        if (canDrop)
        {
            if (item->getQuantity() > 1)
                mBrowserBox->addRow(strprintf("@@drop|%s@@", _("Drop...")));
            else
                mBrowserBox->addRow(strprintf("@@drop|%s@@", _("Drop")));
        }

        if (Net::getInventoryHandler()->canSplit(item))
        {
            mBrowserBox->addRow(strprintf("@@split|%s@@", _("Split")));
        }
    }
    // Assume in storage for now
    // TODO: make this whole system more flexible, if needed
    else
    {
        mBrowserBox->addRow(strprintf("@@retrieve|%s@@", _("Retrieve")));
    }
    mBrowserBox->addRow(strprintf("@@chat|%s@@", _("Add to chat")));
    mBrowserBox->addRow("##3---");
    mBrowserBox->addRow(strprintf("@@cancel|%s@@", _("Cancel")));

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
