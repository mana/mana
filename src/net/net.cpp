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

#include "main.h"
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
#include "net/specialhandler.h"
#include "net/tradehandler.h"

#include "net/manaserv/generalhandler.h"

#include "utils/gettext.h"

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

void connectToServer(ServerInfo &server)
{
    if (server.type == ServerInfo::UNKNOWN)
    {
        // TODO: Query the server about itself and choose the netcode based on
        // that
        if (server.port == 6901)
            server.type = ServerInfo::TMWATHENA;
        else if (server.port == 9601)
            server.type = ServerInfo::MANASERV;
        else
            logger->error(_("Unknown Server Type! Exiting."));
    }

    if (networkType == server.type && getGeneralHandler() != NULL)
    {
        getGeneralHandler()->reload();
    }
    else
    {
        if (networkType != ServerInfo::UNKNOWN && getGeneralHandler() != NULL)
        {
            getGeneralHandler()->unload();
        }

        switch (server.type)
        {
            case ServerInfo::MANASERV:
                new ManaServ::GeneralHandler;
                break;
            default:
                logger->error(_("Server protocol unsupported"));
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
    GeneralHandler *handler = getGeneralHandler();
    if (handler)
    {
        handler->unload();
    }
}

ServerInfo::Type getNetworkType()
{
    return networkType;
}

} // namespace Net

