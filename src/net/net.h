/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef NET_H
#define NET_H

/**
 * \namespace Net
 *
 * The network communication layer. It is composed of a host of interfaces that
 * interact with different aspects of the game. They have different
 * implementations depending on the type of server the client is connecting to.
 */

#include "net/serverinfo.h"

namespace Net {

class AdminHandler;
class CharHandler;
class ChatHandler;
class GameHandler;
class GeneralHandler;
class GuildHandler;
class InventoryHandler;
class LoginHandler;
class NpcHandler;
class PartyHandler;
class PlayerHandler;
class SpecialHandler;
class TradeHandler;

AdminHandler *getAdminHandler();
CharHandler *getCharHandler();
ChatHandler *getChatHandler();
GameHandler *getGameHandler();
GeneralHandler *getGeneralHandler();
GuildHandler *getGuildHandler();
InventoryHandler *getInventoryHandler();
LoginHandler *getLoginHandler();
NpcHandler *getNpcHandler();
PartyHandler *getPartyHandler();
PlayerHandler *getPlayerHandler();
SpecialHandler *getSpecialHandler();
TradeHandler *getTradeHandler();

ServerInfo::Type getNetworkType();

/**
 * Handles server detection and connection
 */
void connectToServer(const ServerInfo &server);

void unload();

} // namespace Net

#endif // NET_H
