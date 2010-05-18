/*
 *  The Mana Client
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

#ifndef NET_TA_GUILDHANDLER_H
#define NET_TA_GUILDHANDLER_H

#include "net/guildhandler.h"

#include "net/tmwa/messagehandler.h"

namespace TmwAthena {

class GuildHandler : public Net::GuildHandler, public MessageHandler
{
    public:
        GuildHandler();

        ~GuildHandler();

        void handleMessage(Net::MessageIn &msg);

        void create(const std::string &name);

        void invite(int guildId, const std::string &name);

        void invite(int guildId, Being *being);

        void inviteResponse(int guildId, bool response);

        void leave(int guildId);

        void kick(GuildMember *member, std::string reason = "");

        void chat(int guildId, const std::string &text);

        void memberList(int guildId);

        void changeMemberPostion(GuildMember *member, int level);

        void requestAlliance(int guildId, int otherGuildId);

        void requestAllianceResponse(int guildId, int otherGuildId,
                                     bool response);

        void endAlliance(int guildId, int otherGuildId);

    private:
        // TmwAthena (and eAthena) only supports one guild per player
        Guild *mGuild;
};

}

#endif // NET_TA_GUILDHANDLER_H
