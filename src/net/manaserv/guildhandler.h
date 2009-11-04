/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
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

#ifndef NET_MANASERV_GUILDHANDLER_H
#define NET_MANASERV_GUILDHANDLER_H

#include "net/guildhandler.h"

#include "net/manaserv/messagehandler.h"

namespace ManaServ {

class GuildHandler : public Net::GuildHandler, public MessageHandler
{
public:
    GuildHandler();

    void handleMessage(Net::MessageIn &msg);

    void create(const std::string &name);

    void invite(int guildId, const std::string &name);

    void invite(int guidId, Player *player);

    void inviteResponse(int guidId, bool response);

    void leave(int guildId);

    void kick(int guildId, int playerId);

    void chat(int guildId, const std::string &text);

    void memberList(int guildId);

    void changeMemberPostion(int guildId, int playerId, int level);

    void requestAlliance(int guildId, int otherGuildId);

    void requestAllianceResponse(int guildId, int otherGuildId,
                                 bool response);

    void endAlliance(int guildId, int otherGuildId);

protected:
    void joinedGuild(Net::MessageIn &msg);
};

} // namespace ManaServ

#endif
