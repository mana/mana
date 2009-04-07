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

namespace Net {

class AdminHandler;
class CharHandler;
class ChatHandler;
class GeneralHandler;
class GuildHandler;
class InventoryHandler;
class LoginHandler;
class LogoutHandler;
class MapHandler;
class NpcHandler;
class PartyHandler;
class PlayerHandler;
class SkillHandler;
class TradeHandler;

AdminHandler *getAdminHandler();
CharHandler *getCharHandler();
ChatHandler *getChatHandler();
GeneralHandler *getGeneralHandler();
GuildHandler *getGuildHandler();
InventoryHandler *getInventoryHandler();
LoginHandler *getLoginHandler();
LogoutHandler *getLogoutHandler();
MapHandler *getMapHandler();
NpcHandler *getNpcHandler();
PartyHandler *getPartyHandler();
PlayerHandler *getPlayerHandler();
SkillHandler *getSkillHandler();
TradeHandler *getTradeHandler();

} // namespace Net

#endif // NET_H
