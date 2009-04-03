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

#include "net/adminhandler.h"
#include "net/charhandler.h"
#include "net/chathandler.h"
#include "net/generalhandler.h"
#include "net/guildhandler.h"
#include "net/inventoryhandler.h"
#include "net/loginhandler.h"
#include "net/maphandler.h"
#include "net/npchandler.h"
#include "net/partyhandler.h"
#include "net/playerhandler.h"
#include "net/skillhandler.h"
#include "net/tradehandler.h"

#ifdef TMWSERV_SUPPORT
#include "net/tmwserv/inventoryhandler.h"
#include "net/tmwserv/playerhandler.h"
#include "net/tmwserv/tradehandler.h"
#else
#include "net/ea/inventoryhandler.h"
#include "net/ea/playerhandler.h"
#include "net/ea/tradehandler.h"
#endif

Net::AdminHandler *Net::getAdminHandler()
{
    // TODO
}

Net::CharHandler *Net::getCharHandler()
{
    // TODO
}

Net::ChatHandler *Net::getChatHandler()
{
    // TODO
}

Net::GeneralHandler *Net::getGeneralHandler()
{
    // TODO
}

Net::GuildHandler *Net::getGuildHandler()
{
    // TODO
}

Net::InventoryHandler *Net::getInventoryHandler()
{
    return inventoryHandler;
}

Net::LoginHandler *Net::getLoginHandler()
{
    // TODO
}

Net::MapHandler *Net::getMapHandler()
{
    // TODO
}

Net::NpcHandler *Net::getNpcHandler()
{
    // TODO
}

Net::PartyHandler *Net::getPartyHandler()
{
    // TODO
}

Net::PlayerHandler *Net::getPlayerHandler()
{
    return playerHandler;
}

Net::SkillHandler *Net::getSkillHandler()
{
    // TODO
}

Net::TradeHandler *Net::getTradeHandler()
{
    return tradeHandler;
}
