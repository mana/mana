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

#ifdef TMWSERV_SUPPORT
#include "net/tmwserv/generalhandler.h"
#else
#include "net/ea/generalhandler.h"
#endif

extern Net::AdminHandler *adminHandler;
extern Net::CharHandler *charHandler;
extern Net::ChatHandler *chatHandler;
extern Net::GeneralHandler *generalHandler;
extern Net::InventoryHandler *inventoryHandler;
extern Net::LoginHandler *loginHandler;
extern Net::GameHandler *gameHandler;
extern Net::NpcHandler *npcHandler;
extern Net::PartyHandler *partyHandler;
extern Net::PlayerHandler *playerHandler;
extern Net::SpecialHandler *specialHandler;
extern Net::TradeHandler *tradeHandler;

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
    // TODO
    return 0;
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

void Net::connectToServer(const ServerInfo &server)
{
    // TODO: Actually query the server about itself and choose the netcode
    // based on that

#ifdef TMWSERV_SUPPORT
    new TmwServ::GeneralHandler;
#else
    new EAthena::GeneralHandler;
#endif

    Net::getGeneralHandler()->load();

    Net::getLoginHandler()->setServer(server);

    Net::getLoginHandler()->connect();
}
