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

#ifndef NET_H
#define NET_H

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

namespace Net {
Net::AdminHandler *getAdminHandler();

Net::CharHandler *getCharHandler();

Net::ChatHandler *getChatHandler();

Net::GeneralHandler *getGeneralHandler();

Net::GuildHandler *getGuildHandler();

Net::InvyHandler *getInvyHandler();

Net::LoginHandler *getLoginHandler();

Net::MapHandler *getMapHandler();

Net::NpcHandler *getNpcHandler();

Net::PartyHandler *getPartyHandler();

Net::PlayerHandler *getPlayerHandler();

Net::SkillHandler *getSkillHandler();

Net::TradeHandler *getTradeHandler();
}

#endif // NET_H
