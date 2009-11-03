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

/**
 * \defgroup Network Core network layer
 */

class ServerInfo;

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

/**
 * Handles server detection and connection
 */
void connectToServer(const ServerInfo &server);

} // namespace Net

#endif // NET_H
