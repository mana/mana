/*
 *  The Mana Client
 *  Copyright (C) 2026  The Mana Developers
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

#include "net/offlinehandler.h"

#include "client.h"
#include "configuration.h"
#include "effectmanager.h"
#include "event.h"
#include "game.h"
#include "inventory.h"
#include "item.h"
#include "localplayer.h"
#include "log.h"
#include "map.h"

#include "gui/skilldialog.h"
#include "gui/statuswindow.h"
#include "gui/widgets/chattab.h"

#include "net/net.h"

#include "net/tmwa/protocol.h"

#include "resources/emotedb.h"
#include "resources/hairdb.h"
#include "resources/itemdb.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/time.h"
#include "utils/xml.h"

#include <cstdlib>
#include <list>
#include <sstream>

using namespace TmwAthena;

extern HairDB hairDB;

extern Net::GeneralHandler *generalHandler;
extern Net::AdminHandler *adminHandler;
extern Net::CharHandler *charHandler;
extern Net::ChatHandler *chatHandler;
extern Net::GameHandler *gameHandler;
extern Net::GuildHandler *guildHandler;
extern Net::InventoryHandler *inventoryHandler;
extern Net::LoginHandler *loginHandler;
extern Net::NpcHandler *npcHandler;
extern Net::PartyHandler *partyHandler;
extern Net::PlayerHandler *playerHandler;
extern Net::AbilityHandler *abilityHandler;
extern Net::TradeHandler *tradeHandler;

static std::string offlineStatePath()
{
    return Client::getConfigDirectory() + "/offline.xml";
}

// --- OfflineHandler ---

OfflineHandler::OfflineHandler()
{
    generalHandler = this;
    adminHandler = this;
    charHandler = this;
    chatHandler = this;
    gameHandler = this;
    guildHandler = this;
    inventoryHandler = this;
    loginHandler = this;
    npcHandler = this;
    partyHandler = this;
    playerHandler = this;
    abilityHandler = this;
    tradeHandler = this;

    std::list<ItemStat> stats;
    stats.emplace_back("str", _("Strength %+d"));
    stats.emplace_back("agi", _("Agility %+d"));
    stats.emplace_back("vit", _("Vitality %+d"));
    stats.emplace_back("int", _("Intelligence %+d"));
    stats.emplace_back("dex", _("Dexterity %+d"));
    stats.emplace_back("luck", _("Luck %+d"));

    setStatsList(std::move(stats));

    listen(Event::ClientChannel);
    listen(Event::GameChannel);
    listen(Event::ItemChannel);
}

// --- GeneralHandler / EventListener ---

void OfflineHandler::event(Event::Channel channel, const Event &event)
{
    if (channel == Event::GameChannel)
    {
        if (event.getType() == Event::Destructing)
        {
            saveState();
        }
        else if (event.getType() == Event::EnginesInitialized)
        {
            Game *game = Game::instance();
            game->changeMap(mMap);

            if (Map *map = game->getCurrentMap())
            {
                if (mTileX && mTileY)
                {
                    local_player->setPosition(map->getTileCenter(mTileX, mTileY));
                    mTileX = mTileY = 0;
                }
                else
                {
                    const int mapW = map->getWidth();
                    const int mapH = map->getHeight();
                    for (int attempts = 0; attempts < 1000; ++attempts)
                    {
                        int x = std::rand() % mapW;
                        int y = std::rand() % mapH;
                        if (map->getWalk(x, y))
                        {
                            local_player->setPosition(map->getTileCenter(x, y));
                            break;
                        }
                    }
                }
            }

            restoreInventory();
        }
        else if (event.getType() == Event::GuiWindowsLoaded)
        {
            skillDialog->loadSkills();

            statusWindow->addAttribute(STRENGTH, _("Strength"), true, "");
            statusWindow->addAttribute(AGILITY, _("Agility"), true, "");
            statusWindow->addAttribute(VITALITY, _("Vitality"), true, "");
            statusWindow->addAttribute(INTELLIGENCE, _("Intelligence"),
                                       true, "");
            statusWindow->addAttribute(DEXTERITY, _("Dexterity"), true, "");
            statusWindow->addAttribute(LUCK, _("Luck"), true, "");

            statusWindow->addAttribute(ATK, _("Attack"), false, "");
            statusWindow->addAttribute(DEF, _("Defense"), false, "");
            statusWindow->addAttribute(MATK, _("M.Attack"), false, "");
            statusWindow->addAttribute(MDEF, _("M.Defense"), false, "");
            //xgettext:no-c-format
            statusWindow->addAttribute(HIT, _("% Accuracy"), false, "");
            //xgettext:no-c-format
            statusWindow->addAttribute(FLEE, _("% Evade"), false, "");
            //xgettext:no-c-format
            statusWindow->addAttribute(CRIT, _("% Critical"), false, "");
        }
    }
    else if (channel == Event::ItemChannel)
    {
        const int invIndex = event.getInt("itemInvIndex", -1);
        const int itemId = event.getInt("itemId", -1);
        if (invIndex < 0 || itemId < 0)
            return;

        Inventory *inventory = PlayerInfo::getInventory();
        if (!inventory)
            return;

        Item *item = inventory->getItem(invIndex);
        if (!item)
            return;

        if (event.getType() == Event::DoEquip)
        {
            int slot = getSlotForItemType(item->getInfo().type);
            if (slot < 0)
            {
                serverNotice(_("Cannot equip this item."));
                return;
            }
            mEquips.setEquipment(slot, invIndex);
        }
        else if (event.getType() == Event::DoUnequip)
        {
            for (int i = 0; i < EQUIP_VECTOR_END; i++)
            {
                if (mEquips.getEquipmentIndex(i) == invIndex)
                {
                    mEquips.setEquipment(i, -1);
                    break;
                }
            }
        }
    }
}

// --- GameHandler ---

void OfflineHandler::connect()
{
    Client::setState(State::Game);
}

void OfflineHandler::quit()
{
    Client::setState(State::Exit);
}

int OfflineHandler::getPickupRange() const
{
    if (Game *game = Game::instance())
        return game->getCurrentTileWidth();
    return DEFAULT_TILE_LENGTH;
}

int OfflineHandler::getNpcTalkRange() const
{
    return DEFAULT_TILE_LENGTH * 30;
}

void OfflineHandler::setMap(const std::string &map)
{
    mMap = map.substr(0, map.rfind('.'));

    // Explicitly requesting a map discards any restored position
    mTileX = mTileY = 0;
}

bool OfflineHandler::loadSavedState()
{
    const std::string path = offlineStatePath();
    XML::Document doc(path, false);

    XML::Node rootNode = doc.rootNode();
    if (!rootNode || rootNode.name() != "offline")
        return false;

    for (auto child : rootNode.children())
    {
        if (child.name() == "location")
        {
            std::string map = child.getProperty("map", std::string());
            int x = child.getProperty("x", 0);
            int y = child.getProperty("y", 0);

            if (!map.empty())
                setMap(map);

            if (x > 0 && y > 0)
                setTilePosition(x, y);
        }
        else if (child.name() == "inventory")
        {
            for (auto itemNode : child.children())
            {
                if (itemNode.name() != "item")
                    continue;

                SavedItem item;
                itemNode.attribute("slot", item.slot);
                itemNode.attribute("id", item.id);
                itemNode.attribute("amount", item.amount);

                bool equipped = false;
                itemNode.attribute("equipped", equipped);
                if (equipped)
                    itemNode.attribute("equipSlot", item.equipSlot);

                if (item.slot >= 0 && item.id > 0)
                    mSavedInventory.push_back(item);
            }
        }
    }

    Log::info("Loaded offline state from %s", path.c_str());
    return true;
}

// --- ChatHandler ---

void OfflineHandler::talk(const std::string &text)
{
    if (!text.empty() && text[0] == '@')
    {
        handleCommand(text.substr(1));
        return;
    }

    if (localChatTab && local_player)
        localChatTab->chatLog(local_player->getName(), text);
}

void OfflineHandler::me(const std::string &text)
{
    if (localChatTab && local_player)
    {
        localChatTab->chatLog(strprintf("* %s %s",
                              local_player->getName().c_str(),
                              text.c_str()));
    }
}

void OfflineHandler::privateMessage(const std::string &recipient,
                                    const std::string &text)
{
    serverNotice(strprintf(_("Cannot send a private message to %s in offline "
                             "mode."), recipient.c_str()));
}

void OfflineHandler::handleCommand(const std::string &text)
{
    std::istringstream iss(text);
    std::string cmd;
    iss >> cmd;

    if (cmd == "item")
    {
        int id = 0;
        int amount = 1;
        iss >> id;
        if (iss.good())
            iss >> amount;

        if (id <= 0)
        {
            serverNotice(strprintf(_("Usage: %s"), "@item <id> [amount]"));
            return;
        }

        if (amount < 1)
            amount = 1;

        Inventory *inventory = PlayerInfo::getInventory();
        if (!inventory)
            return;

        int freeSlot = -1;
        for (int i = 0; i < static_cast<int>(getSize(Inventory::INVENTORY)); ++i)
        {
            Item *existing = inventory->getItem(i);
            if (existing && existing->getId() == id)
            {
                freeSlot = i;
                amount += existing->getQuantity();
                break;
            }
        }

        if (freeSlot < 0)
        {
            for (int i = 0; i < static_cast<int>(getSize(Inventory::INVENTORY)); ++i)
            {
                Item *existing = inventory->getItem(i);
                if (!existing || existing->getId() == 0)
                {
                    freeSlot = i;
                    break;
                }
            }
        }

        if (freeSlot < 0)
        {
            serverNotice(_("Inventory is full."));
            return;
        }

        PlayerInfo::setInventoryItem(freeSlot, id, amount);
        serverNotice(strprintf(_("Slot %d now holds %d of item %d."),
                               freeSlot, amount, id));
    }
    else if (cmd == "jump")
    {
        int x = 0, y = 0;
        iss >> x >> y;

        if (x <= 0 || y <= 0)
        {
            serverNotice(strprintf(_("Usage: %s"), "@jump <x> <y>"));
            return;
        }

        Game *game = Game::instance();
        if (!game)
            return;

        Map *map = game->getCurrentMap();
        if (!map)
            return;

        local_player->setPosition(map->getTileCenter(x, y));
        local_player->setDestination(map->getTileCenter(x, y).x,
                                     map->getTileCenter(x, y).y);
        serverNotice(strprintf(_("Jumped to %d, %d."), x, y));
    }
    else if (cmd == "map")
    {
        std::string mapName;
        int x = 0, y = 0;
        iss >> mapName;

        if (mapName.empty())
        {
            serverNotice(strprintf(_("Usage: %s"), "@map <mapname> [x] [y]"));
            return;
        }

        if (iss.good())
            iss >> x;
        if (iss.good())
            iss >> y;

        Game *game = Game::instance();
        if (!game)
            return;

        local_player->stopAttack();

        auto dotPos = mapName.rfind('.');
        if (dotPos != std::string::npos)
            mapName = mapName.substr(0, dotPos);

        game->changeMap(mapName);

        if (Map *map = game->getCurrentMap())
        {
            if (x > 0 && y > 0)
            {
                local_player->setPosition(map->getTileCenter(x, y));
            }
            else
            {
                const int mapW = map->getWidth();
                const int mapH = map->getHeight();
                for (int attempts = 0; attempts < 1000; ++attempts)
                {
                    int rx = std::rand() % mapW;
                    int ry = std::rand() % mapH;
                    if (map->getWalk(rx, ry))
                    {
                        local_player->setPosition(map->getTileCenter(rx, ry));
                        break;
                    }
                }
            }
        }

        serverNotice(strprintf(_("Changed map to %s."), mapName.c_str()));
    }
    else if (cmd == "zeny")
    {
        int amount = 0;
        iss >> amount;

        if (!iss || amount < 0)
        {
            serverNotice(strprintf(_("Usage: %s"), "@zeny <amount>"));
            return;
        }

        PlayerInfo::setAttribute(MONEY, amount);
        serverNotice(strprintf(_("Money set to %d."), amount));
    }
    else if (cmd == "die")
    {
        PlayerInfo::setAttribute(HP, 0);
        local_player->setAction(Being::DEAD);
        serverNotice(_("You are dead."));
    }
    else if (cmd == "alive")
    {
        PlayerInfo::setAttribute(HP, PlayerInfo::getAttribute(MAX_HP));
        local_player->setAction(Being::STAND);
        serverNotice(_("You are alive again."));
    }
    else if (cmd == "help")
    {
        // The command names are not translatable, only their descriptions
        serverNotice(_("Available commands:"));
        serverNotice(strprintf("  %s - %s", "@item <id> [amount]",
                               _("Add an item to the inventory")));
        serverNotice(strprintf("  %s - %s", "@jump <x> <y>",
                               _("Teleport to a position")));
        serverNotice(strprintf("  %s - %s", "@map <mapname> [x] [y]",
                               _("Change map")));
        serverNotice(strprintf("  %s - %s", "@zeny <amount>",
                               _("Set the amount of money")));
        serverNotice(strprintf("  %s - %s", "@die", _("Kill yourself")));
        serverNotice(strprintf("  %s - %s", "@alive", _("Revive yourself")));
        serverNotice(strprintf("  %s - %s", "@help", _("Show this help")));
    }
    else
    {
        serverNotice(strprintf(_("Unknown command: %s. Type %s for a list of "
                                 "commands."), ("@" + cmd).c_str(), "@help"));
    }
}

// --- InventoryHandler ---

size_t OfflineHandler::getSize(int type) const
{
    switch (type)
    {
        case Inventory::INVENTORY:
            return 100;
        case Inventory::STORAGE:
        default:
            return 0;
    }
}

bool OfflineHandler::isWeaponSlot(unsigned int slotTypeId) const
{
    return (slotTypeId == EQUIP_FIGHT1_SLOT ||
            slotTypeId == EQUIP_FIGHT2_SLOT);
}

bool OfflineHandler::isAmmoSlot(unsigned int slotTypeId) const
{
    return slotTypeId == EQUIP_PROJECTILE_SLOT;
}

int OfflineHandler::getSlotForItemType(ItemType type) const
{
    switch (type)
    {
        case ITEM_EQUIPMENT_TORSO:              return EQUIP_TORSO_SLOT;
        case ITEM_EQUIPMENT_HEAD:               return EQUIP_HEAD_SLOT;
        case ITEM_EQUIPMENT_LEGS:               return EQUIP_LEGS_SLOT;
        case ITEM_EQUIPMENT_FEET:               return EQUIP_FEET_SLOT;
        case ITEM_EQUIPMENT_ARMS:               return EQUIP_ARMS_SLOT;
        case ITEM_EQUIPMENT_RING:
            if (mEquips.getEquipmentIndex(EQUIP_RING1_SLOT) == -1)
                return EQUIP_RING1_SLOT;
            return EQUIP_RING2_SLOT;
        case ITEM_EQUIPMENT_NECKLACE:           return EQUIP_NECKLACE_SLOT;
        case ITEM_EQUIPMENT_ONE_HAND_WEAPON:
        case ITEM_EQUIPMENT_TWO_HANDS_WEAPON:   return EQUIP_FIGHT1_SLOT;
        case ITEM_EQUIPMENT_SHIELD:             return EQUIP_FIGHT2_SLOT;
        case ITEM_EQUIPMENT_AMMO:               return EQUIP_PROJECTILE_SLOT;
        default:                                return -1;
    }
}

// --- PlayerHandler ---

void OfflineHandler::emote(int emoteId)
{
    if (local_player && effectManager)
    {
        const int fx = EmoteDB::get(emoteId).effectId;
        effectManager->trigger(fx, local_player);
    }
}

void OfflineHandler::setDirection(char direction)
{
    if (local_player)
        local_player->setDirection(direction);
}

void OfflineHandler::setDestination(int x, int y, int direction)
{
    if (local_player)
        local_player->setDestination(x, y);
}

void OfflineHandler::changeAction(Being::Action action)
{
    if (local_player)
        local_player->setAction(action);
}

int OfflineHandler::getJobLocation()
{
    return JOB;
}

Vector OfflineHandler::getDefaultMoveSpeed() const
{
    return { 15.0f, 15.0f, 0.0f };
}

Vector OfflineHandler::getPixelsPerSecondMoveSpeed(const Vector &speed, Map *map)
{
    Game *game = Game::instance();

    if (game && !map)
        map = game->getCurrentMap();

    if (!map || speed.x == 0 || speed.y == 0)
        return getDefaultMoveSpeed();

    constexpr float ticksPerSecond = 1000.0 / MILLISECONDS_IN_A_TICK;

    return {
        map->getTileWidth() / speed.x * ticksPerSecond,
        map->getTileHeight() / speed.y * ticksPerSecond,
        0.0f
    };
}

// --- CharHandler ---

void OfflineHandler::requestCharacters()
{
    local_player = new LocalPlayer(1, 0);
    local_player->setName(config.lastCharacter.empty() ? "Player"
                                                       : config.lastCharacter);
    local_player->setGender(Gender::Male);
    local_player->setSprite(SPRITE_HAIR, -1, hairDB.getHairColor(0));
    PlayerInfo::setBackend(PlayerInfoBackend());

    PlayerInfo::setAttribute(LEVEL, 99);
    PlayerInfo::setAttribute(HP, 9999);
    PlayerInfo::setAttribute(MAX_HP, 9999);
    PlayerInfo::setAttribute(MP, 999);
    PlayerInfo::setAttribute(MAX_MP, 999);

    Client::setState(State::Game);
}

// --- Persistence ---

void OfflineHandler::saveState()
{
    if (!local_player)
        return;

    const std::string path = offlineStatePath();
    XML::Writer writer(path);

    if (!writer.isValid())
    {
        Log::warn("Could not save offline state to %s", path.c_str());
        return;
    }

    writer.startElement("offline");

    if (Game *game = Game::instance())
    {
        if (game->getCurrentMap())
        {
            writer.startElement("location");
            writer.addAttribute("map", game->getCurrentMapName());
            writer.addAttribute("x", local_player->getTileX());
            writer.addAttribute("y", local_player->getTileY());
            writer.endElement();
        }
    }

    if (Inventory *inventory = PlayerInfo::getInventory())
    {
        writer.startElement("inventory");

        for (int i = 0; i < static_cast<int>(getSize(Inventory::INVENTORY)); ++i)
        {
            Item *item = inventory->getItem(i);
            if (!item || item->getId() == 0)
                continue;

            writer.startElement("item");
            writer.addAttribute("slot", i);
            writer.addAttribute("id", item->getId());
            writer.addAttribute("amount", item->getQuantity());

            if (item->isEquipped())
            {
                writer.addAttribute("equipped", true);
                for (int s = 0; s < EQUIP_VECTOR_END; ++s)
                {
                    if (mEquips.getEquipmentIndex(s) == i)
                    {
                        writer.addAttribute("equipSlot", s);
                        break;
                    }
                }
            }

            writer.endElement();
        }

        writer.endElement();
    }

    writer.endElement();

    Log::info("Saved offline state to %s", path.c_str());
}

void OfflineHandler::restoreInventory()
{
    for (const SavedItem &item : mSavedInventory)
    {
        PlayerInfo::setInventoryItem(item.slot, item.id, item.amount);

        if (item.equipSlot >= 0)
            mEquips.setEquipment(item.equipSlot, item.slot);
    }

    mSavedInventory.clear();
}
