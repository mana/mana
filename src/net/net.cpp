/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "net/net.h"

#include "log.h"

#include "net/adminhandler.h"
#include "net/charhandler.h"
#include "net/chathandler.h"
#include "net/generalhandler.h"
#include "net/guildhandler.h"
#include "net/inventoryhandler.h"
#include "net/loginhandler.h"
#include "net/gamehandler.h"
#include "net/npchandler.h"
#include "net/partyhandler.h"
#include "net/playerhandler.h"
#include "net/abilityhandler.h"
#include "net/tradehandler.h"

#include "net/tmwa/generalhandler.h"

#include "net/manaserv/generalhandler.h"

#include "utils/gettext.h"

Net::AdminHandler *adminHandler = nullptr;
Net::CharHandler *charHandler = nullptr;
Net::ChatHandler *chatHandler = nullptr;
Net::GeneralHandler *generalHandler = nullptr;
Net::InventoryHandler *inventoryHandler = nullptr;
Net::LoginHandler *loginHandler = nullptr;
Net::GameHandler *gameHandler = nullptr;
Net::GuildHandler *guildHandler = nullptr;
Net::NpcHandler *npcHandler = nullptr;
Net::PartyHandler *partyHandler = nullptr;
Net::PlayerHandler *playerHandler = nullptr;
Net::AbilityHandler *abilityHandler = nullptr;
Net::TradeHandler *tradeHandler = nullptr;

Net::AdminHandler *Net::getAdminHandler()
{
    return adminHandler;
}

Net::CharHandler *Net::getCharHandler()
{
    return charHandler;
}

Net::ChatHandler *Net::getChatHandler()
{
    return chatHandler;
}

Net::GameHandler *Net::getGameHandler()
{
    return gameHandler;
}

Net::GeneralHandler *Net::getGeneralHandler()
{
    return generalHandler;
}

Net::GuildHandler *Net::getGuildHandler()
{
    return guildHandler;
}

Net::InventoryHandler *Net::getInventoryHandler()
{
    return inventoryHandler;
}

Net::LoginHandler *Net::getLoginHandler()
{
    return loginHandler;
}

Net::NpcHandler *Net::getNpcHandler()
{
    return npcHandler;
}

Net::PartyHandler *Net::getPartyHandler()
{
    return partyHandler;
}

Net::PlayerHandler *Net::getPlayerHandler()
{
    return playerHandler;
}

Net::AbilityHandler *Net::getAbilityHandler()
{
    return abilityHandler;
}

Net::TradeHandler *Net::getTradeHandler()
{
    return tradeHandler;
}

namespace Net
{

static ServerType networkType = ServerType::Unknown;

void connectToServer(ServerInfo &server)
{
    if (server.type == ServerType::Unknown)
    {
        // TODO: Query the server about itself and choose the netcode based on
        // that
        if (server.port == 6901)
            server.type = ServerType::TmwAthena;
        else if (server.port == 9601)
            server.type = ServerType::ManaServ;
        else
            Log::critical(_("Unknown Server Type! Exiting."));
    }

    if (networkType == server.type && getGeneralHandler() != nullptr)
    {
        getGeneralHandler()->reload();
    }
    else
    {
        unload();

        switch (server.type)
        {
#ifdef MANASERV_SUPPORT
            case ServerType::ManaServ:
                generalHandler = new ManaServ::GeneralHandler;
                break;
#endif
            case ServerType::TmwAthena:
                generalHandler = new TmwAthena::GeneralHandler;
                break;
            default:
                Log::critical(_("Server protocol unsupported"));
                break;
        }

        getGeneralHandler()->load();

        networkType = server.type;
    }

    getLoginHandler()->setServer(server);

    getLoginHandler()->connect();
}

void unload()
{
    if (!generalHandler)
        return;

    generalHandler->unload();
    delete generalHandler;

    adminHandler = nullptr;
    charHandler = nullptr;
    chatHandler = nullptr;
    generalHandler = nullptr;
    inventoryHandler = nullptr;
    loginHandler = nullptr;
    gameHandler = nullptr;
    guildHandler = nullptr;
    npcHandler = nullptr;
    partyHandler = nullptr;
    playerHandler = nullptr;
    abilityHandler = nullptr;
    tradeHandler = nullptr;
}

ServerType getNetworkType()
{
    return networkType;
}

} // namespace Net
