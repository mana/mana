/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "net/net.h"

#include "main.h"

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
#include "net/specialhandler.h"
#include "net/tradehandler.h"

#include "net/ea/generalhandler.h"

#include "net/manaserv/generalhandler.h"

Net::AdminHandler *adminHandler = NULL;
Net::CharHandler *charHandler = NULL;
Net::ChatHandler *chatHandler = NULL;
Net::GeneralHandler *generalHandler = NULL;
Net::InventoryHandler *inventoryHandler = NULL;
Net::LoginHandler *loginHandler = NULL;
Net::GameHandler *gameHandler = NULL;
Net::GuildHandler *guildHandler = NULL;
Net::NpcHandler *npcHandler = NULL;
Net::PartyHandler *partyHandler = NULL;
Net::PlayerHandler *playerHandler = NULL;
Net::SpecialHandler *specialHandler = NULL;
Net::TradeHandler *tradeHandler = NULL;

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

Net::SpecialHandler *Net::getSpecialHandler()
{
    return specialHandler;
}

Net::TradeHandler *Net::getTradeHandler()
{
    return tradeHandler;
}

namespace Net
{
ServerInfo::Type networkType = ServerInfo::UNKNOWN;

void connectToServer(const ServerInfo &server)
{
    // Remove with ifdefs
    if (networkType != ServerInfo::UNKNOWN)
    {
        getGeneralHandler()->reload();
    }
    else
    {
#ifdef MANASERV_SUPPORT
        new ManaServ::GeneralHandler;
#else
        new EAthena::GeneralHandler;
#endif

        getGeneralHandler()->load();

        networkType = server.type;
    }
    // End remove section

    // Uncomment after ifdefs removed
    /*ServerInfo server = ServerInfo(inServer);
    if (server.type == ServerInfo::UNKNOWN)
    {
        // TODO: Query the server about itself and choose the netcode based on
        // that
    }

    //if (networkType == server.type)
    if (networkType != ServerInfo::UNKNOWN)
    {
        getGeneralHandler()->reload();
    }
    else
    {
        if (networkType != ServerInfo::UNKNOWN)
        {
            getGeneralHandler()->unload();
        }

        switch (server.type)
        {
            case ServerInfo::MANASERV:
                new ManaServ::GeneralHandler;
                break;

            case ServerInfo::EATHENA:
                new EAthena::GeneralHandler;
                break;

            default:
                // Shouldn't happen...
                break;
        }

        getGeneralHandler()->load();

        networkType = server.type;
    }*/

    getLoginHandler()->setServer(server);

    getLoginHandler()->connect();
}

void unload()
{
    GeneralHandler *handler = getGeneralHandler();
    if (handler)
    {
        handler->unload();
    }
}

} // namespace Net