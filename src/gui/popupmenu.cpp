/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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
#include "item.h"
#include "localplayer.h"
#include "playerinfo.h"
#include "playerrelations.h"

#include "gui/chatwindow.h"
#include "gui/equipmentwindow.h"
#include "gui/inventorywindow.h"
#include "gui/itemamountwindow.h"

#include "net/adminhandler.h"
#include "net/net.h"
#include "net/partyhandler.h"
#include "net/tradehandler.h"

#include "resources/iteminfo.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <cassert>

std::string tradePartnerName;

/**
 * Wraps an action on a being into one that looks the being up again when it
 * is triggered, so that it is skipped when the being is gone by then.
 */
static Menu::Action onBeing(int beingId, std::function<void (Being *)> action)
{
    return [beingId, action = std::move(action)] {
        if (Being *being = actorSpriteManager->findBeing(beingId))
            action(being);
    };
}

/**
 * Wraps an action on a floor item, see onBeing.
 */
static Menu::Action onFloorItem(int itemId, std::function<void (FloorItem *)> action)
{
    return [itemId, action = std::move(action)] {
        if (FloorItem *floorItem = actorSpriteManager->findItem(itemId))
            action(floorItem);
    };
}

void addPlayerRelationItems(Menu &menu, const std::string &name)
{
    auto setRelation = [name](PlayerRelation relation) {
        return [name, relation] {
            player_relations.setRelation(name, relation);
        };
    };

    switch (player_relations.getRelation(name))
    {
        case PlayerRelation::Neutral:
            menu.addItem(strprintf(_("Befriend %s"), name.c_str()),
                         setRelation(PlayerRelation::Friend));
            [[fallthrough]];

        case PlayerRelation::Friend:
            menu.addItem(strprintf(_("Disregard %s"), name.c_str()),
                         setRelation(PlayerRelation::Disregarded));
            menu.addItem(strprintf(_("Ignore %s"), name.c_str()),
                         setRelation(PlayerRelation::Ignored));
            break;

        case PlayerRelation::Disregarded:
            menu.addItem(strprintf(_("Unignore %s"), name.c_str()),
                         setRelation(PlayerRelation::Neutral));
            menu.addItem(strprintf(_("Completely ignore %s"), name.c_str()),
                         setRelation(PlayerRelation::Ignored));
            break;

        case PlayerRelation::Ignored:
            menu.addItem(strprintf(_("Unignore %s"), name.c_str()),
                         setRelation(PlayerRelation::Neutral));
            break;
    }
}

void PopupMenu::showPopup(int x, int y, Being *being)
{
    clear();

    const int beingId = being->getId();
    const std::string &name = being->getName();

    switch (being->getType())
    {
        case ActorSprite::PLAYER:
        {
            // Players can be traded with.
            addItem(strprintf(_("Trade with %s..."), name.c_str()),
                    onBeing(beingId, [](Being *being) {
                        Net::getTradeHandler()->request(being);
                        tradePartnerName = being->getName();
                    }));
            // TRANSLATORS: Attacking a player.
            addItem(strprintf(_("Attack %s"), name.c_str()),
                    onBeing(beingId, [](Being *being) {
                        local_player->attack(being, true);
                    }));
            // TRANSLATORS: Whispering a player.
            addItem(strprintf(_("Whisper %s"), name.c_str()),
                    onBeing(beingId, [](Being *being) {
                        chatWindow->addInputText("/w \"" + being->getName() + "\" ");
                    }));

            addSeparator();

            addPlayerRelationItems(*this, name);

            if (local_player->getNumberOfGuilds())
            {
                addItem(strprintf(_("Invite %s to join your guild"), name.c_str()),
                        onBeing(beingId, [](Being *being) {
                            local_player->inviteToGuild(being);
                        }));
            }
            if (local_player->isInParty() ||
                Net::getNetworkType() == ServerType::ManaServ)
            {
                addItem(strprintf(_("Invite %s to join your party"), name.c_str()),
                        onBeing(beingId, [](Being *being) {
                            Net::getPartyHandler()->invite(being);
                        }));
            }

            if (local_player->isGM())
            {
                addSeparator();
                addItem(_("Kick player"),
                        onBeing(beingId, [](Being *being) {
                            Net::getAdminHandler()->kick(being->getName());
                        }));
            }
            break;
        }

        case ActorSprite::NPC:
            // NPCs can be talked to (single option, candidate for removal
            // unless more options would be added)
            addItem(strprintf(_("Talk to %s"), name.c_str()),
                    onBeing(beingId, [](Being *being) {
                        if (being->canTalk())
                            being->talkTo();
                    }));
            break;

        case ActorSprite::MONSTER:
            // Monsters can be attacked
            addItem(strprintf(_("Attack %s"), name.c_str()),
                    onBeing(beingId, [](Being *being) {
                        local_player->attack(being, true);
                    }));
            break;

        default:
            /* Other beings aren't interesting... */
            return;
    }

    addItem(_("Add name to chat"),
            onBeing(beingId, [](Being *being) {
                chatWindow->addInputText(being->getName());
            }));

    addSeparator();
    addItem(_("Cancel"));

    showAt(x, y);
}

void PopupMenu::showPopup(int x, int y, FloorItem *floorItem)
{
    clear();

    const int itemId = floorItem->getId();
    const std::string &name = floorItem->getInfo().name;

    // Floor item can be picked up (single option, candidate for removal)
    addItem(strprintf(_("Pick up %s"), name.c_str()),
            onFloorItem(itemId, [](FloorItem *floorItem) {
                local_player->pickUp(floorItem);
            }));
    addItem(_("Add to chat"),
            onFloorItem(itemId, [](FloorItem *floorItem) {
                chatWindow->addItemText(floorItem->getInfo().name);
            }));

    addSeparator();
    addItem(_("Cancel"));

    showAt(x, y);
}

void PopupMenu::showPopup(Window *parent, int x, int y, Item *item,
                          bool isInventory, bool canDrop)
{
    assert(item);
    clear();

    if (isInventory)
    {
        if (PlayerInfo::getStorageCount() > 0)
        {
            addItem(_("Store"), [item] {
                ItemAmountWindow::showWindow(ItemAmountWindow::StoreAdd,
                                             inventoryWindow, item);
            });
        }

        auto &itemInfo = item->getInfo();

        // Equipping, unequipping and activating all end up here, since
        // whether an item is used or (un)equipped depends on the item.
        auto useItem = [item] {
            if (item->isEquippable())
            {
                if (item->isEquipped())
                {
                    PlayerInfo::getEquipment()->triggerUnequip(
                                                equipmentWindow->getSelected());
                }
                else
                {
                    item->doEvent(Event::DoEquip);
                }
            }
            else
            {
                item->doEvent(Event::DoUse);
            }
        };

        if (itemInfo.equippable)
            addItem(item->isEquipped() ? _("Unequip") : _("Equip"), useItem);

        if (itemInfo.activatable)
        {
            addItem(itemInfo.useText.empty()
                        ? _("Activate")
                        : gettext(itemInfo.useText.c_str()), useItem);
        }

        if (canDrop)
        {
            addItem(item->getQuantity() > 1 ? _("Drop...") : _("Drop"), [item] {
                ItemAmountWindow::showWindow(ItemAmountWindow::ItemDrop,
                                             inventoryWindow, item);
            });
        }
    }
    // Assume in storage for now
    // TODO: make this whole system more flexible, if needed
    else
    {
        addItem(_("Retrieve"), [parent, item] {
            ItemAmountWindow::showWindow(ItemAmountWindow::StoreRemove,
                                         parent, item);
        });
    }

    addItem(_("Add to chat"), [item] {
        chatWindow->addItemText(item->getInfo().name);
    });

    addSeparator();
    addItem(_("Cancel"));

    showAt(x, y);
}
