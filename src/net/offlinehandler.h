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

#pragma once

#include "eventlistener.h"

#include "net/abilityhandler.h"
#include "net/adminhandler.h"
#include "net/charhandler.h"
#include "net/chathandler.h"
#include "net/gamehandler.h"
#include "net/generalhandler.h"
#include "net/guildhandler.h"
#include "net/inventoryhandler.h"
#include "net/loginhandler.h"
#include "net/npchandler.h"
#include "net/partyhandler.h"
#include "net/playerhandler.h"
#include "net/tradehandler.h"

#include "net/tmwa/inventoryhandler.h"

#include "resources/iteminfo.h"

#include <vector>

/**
 * A single handler that implements all network handler interfaces for offline
 * mode. Since there is no server, most methods are no-ops. The tmwAthena data
 * model is reused, so its equipment backend serves offline mode as well.
 *
 * Note: LoginHandler and GameHandler both declare connect(), isConnected(),
 * and disconnect() with identical signatures. The single override for each
 * implements GameHandler's semantics, which is fine since the LoginHandler
 * methods are never called in offline mode. Similarly, PartyHandler and
 * GuildHandler both declare create(const std::string&) — the single no-op
 * override satisfies both.
 */
class OfflineHandler final : public Net::GeneralHandler,
                             public Net::GameHandler,
                             public Net::ChatHandler,
                             public Net::InventoryHandler,
                             public Net::PlayerHandler,
                             public Net::LoginHandler,
                             public Net::CharHandler,
                             public Net::AdminHandler,
                             public Net::NpcHandler,
                             public Net::PartyHandler,
                             public Net::GuildHandler,
                             public Net::AbilityHandler,
                             public Net::TradeHandler,
                             public EventListener
{
    public:
        OfflineHandler();

        // --- GeneralHandler ---
        void load() override {}
        void reload() override {}
        void unload() override {}
        void flushNetwork() override {}

        // --- GameHandler ---
        // Note: connect/isConnected/disconnect also satisfy LoginHandler
        void connect() override;
        bool isConnected() override { return true; }
        void disconnect() override {}
        void quit() override;
        bool removeDeadBeings() const override { return true; }
        bool canUseMagicBar() const override { return true; }
        int getPickupRange() const override;
        int getNpcTalkRange() const override;

        // --- ChatHandler ---
        void talk(const std::string &text) override;
        void me(const std::string &text) override;
        void privateMessage(const std::string &recipient,
                            const std::string &text) override;
        void channelList() override {}
        void enterChannel(const std::string &channel,
                          const std::string &password) override {}
        void quitChannel(int channelId) override {}
        void sendToChannel(int channelId, const std::string &text) override {}
        void userList(const std::string &channel) override {}
        void setChannelTopic(int channelId, const std::string &text) override {}
        void setUserMode(int channelId, const std::string &name, int mode) override {}
        void kickUser(int channelId, const std::string &name) override {}
        void who() override {}
        bool whoSupported() const override { return false; }
        void requestOnlineList() override {}

        // --- InventoryHandler ---
        size_t getSize(int type) const override;
        bool isWeaponSlot(unsigned int slotTypeId) const override;
        bool isAmmoSlot(unsigned int slotTypeId) const override;
        Equipment::Backend *getEquipmentBackend() override { return &mEquips; }

        // --- PlayerHandler ---
        void attack(int id) override {}
        void emote(int emoteId) override;
        void increaseAttribute(int attr) override {}
        void decreaseAttribute(int attr) override {}
        void increaseSkill(int skillId) override {}
        void pickUp(FloorItem *floorItem) override {}
        void setDirection(char direction) override;
        void setDestination(int x, int y, int direction = -1) override;
        void changeAction(Being::Action action) override;
        void respawn() override {}
        void ignorePlayer(const std::string &player, bool ignore) override {}
        void ignoreAll(bool ignore) override {}
        bool canUseMagic() override { return false; }
        bool canCorrectAttributes() override { return false; }
        int getJobLocation() override;
        Vector getDefaultMoveSpeed() const override;
        Vector getPixelsPerSecondMoveSpeed(const Vector &speed, Map *map = nullptr) override;
        bool usePixelPrecision() override { return false; }

        // --- LoginHandler ---
        // connect/isConnected/disconnect already declared above
        int supportedOptionalActions() const override { return 0; }
        bool isRegistrationEnabled() override { return false; }
        void getRegistrationDetails() override {}
        void loginAccount(LoginData *loginData) override {}
        void logout() override {}
        void changeEmail(const std::string &email) override {}
        void changePassword(const std::string &username,
                            const std::string &oldPassword,
                            const std::string &newPassword) override {}
        void chooseServer(unsigned int server) override {}
        void registerAccount(LoginData *loginData) override {}
        void unregisterAccount(const std::string &username,
                               const std::string &password) override {}
        Worlds getWorlds() const override { return {}; }

        // --- CharHandler ---
        void setCharSelectDialog(CharSelectDialog *window) override {}
        void setCharCreateDialog(CharCreateDialog *window) override {}
        void requestCharacters() override;
        void chooseCharacter(int slot) override {}
        void newCharacter(const std::string &name, int slot,
                          Gender gender, int hairstyle, int hairColor,
                          const std::vector<int> &stats) override {}
        void deleteCharacter(int slot) override {}
        void switchCharacter() override {}
        unsigned int baseSprite() const override { return 0; }
        unsigned int hairSprite() const override { return 1; }
        unsigned int maxSprite() const override { return 2; }
        int getCharCreateMaxHairColorId() const override { return 0; }
        int getCharCreateMaxHairStyleId() const override { return 0; }

        // --- AdminHandler ---
        void kick(const std::string &name) override {}
        void ban(const std::string &name) override {}
        void unban(const std::string &name) override {}

        // --- NpcHandler ---
        void buy(int beingId) override {}
        void sell(int beingId) override {}
        void buyItem(int beingId, int itemId, int amount) override {}
        void sellItem(int beingId, int itemId, int amount) override {}
        // talk(int) doesn't conflict with ChatHandler::talk(string)
        void talk(int npcId) override {}
        void nextDialog(int npcId) override {}
        void closeDialog(int npcId) override {}
        void cancelDialog(int npcId) override {}
        void menuSelect(int npcId, int choice) override {}
        void integerInput(int npcId, int value) override {}
        void stringInput(int npcId, const std::string &value) override {}
        void sendLetter(int npcId, const std::string &recipient,
                        const std::string &text) override {}

        // --- PartyHandler ---
        // Note: create(string) also satisfies GuildHandler::create(string)
        void create(const std::string &name = std::string()) override {}
        void join(int partyId) override {}
        void invite(Being *player) override {}
        void invite(const std::string &name) override {}
        void inviteResponse(const std::string &inviter, bool accept) override {}
        void leave() override {}
        void kick(Being *player) override {}
        void kick(GuildMember *member, std::string reason = std::string()) override {}
        void chat(const std::string &text) override {}
        void requestPartyMembers() override {}
        PartyShare getShareExperience() override { return PARTY_SHARE_NO; }
        void setShareExperience(PartyShare share) override {}
        PartyShare getShareItems() override { return PARTY_SHARE_NO; }
        void setShareItems(PartyShare share) override {}

        // --- GuildHandler ---
        // create(string) already declared above
        void invite(int guildId, const std::string &name) override {}
        void invite(int guildId, Being *being) override {}
        void inviteResponse(int guildId, bool response) override {}
        void leave(int guildId) override {}
        void chat(int guildId, const std::string &text) override {}
        void memberList(int guildId) override {}
        void changeMemberPostion(GuildMember *member, int level) override {}
        void requestAlliance(int guildId, int otherGuildId) override {}
        void requestAllianceResponse(int guildId, int otherGuildId,
                                     bool response) override {}
        void endAlliance(int guildId, int otherGuildId) override {}

        // --- AbilityHandler ---
        void use(int id) override {}
        void useOn(int id, int beingId) override {}
        void useAt(int id, int x, int y) override {}
        void useInDirection(int id, int direction) override {}

        // --- EventListener ---
        void event(Event::Channel channel, const Event &event) override;

        // --- Offline-specific ---
        void setMap(const std::string &map);
        bool loadSavedState();
        void setTilePosition(int x, int y) { mTileX = x; mTileY = y; }

    private:
        void handleCommand(const std::string &text);
        int getSlotForItemType(ItemType type) const;
        void saveState();
        void restoreInventory();

        struct SavedItem
        {
            int slot = -1;
            int id = 0;
            int amount = 1;
            int equipSlot = -1;
        };

        TmwAthena::EquipBackend mEquips;
        std::string mMap;
        int mTileX = 0;
        int mTileY = 0;
        std::vector<SavedItem> mSavedInventory;
};
