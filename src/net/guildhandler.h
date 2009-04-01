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

#ifndef GUILDHANDLER_H
#define GUILDHANDLER_H

#include <iosfwd>

namespace Net {
class GuildHandler
{
    public:
        virtual void create(const std::string &name) {}

        virtual void join(int guildId) {}

        virtual void invite(int guidId, int playerId) {}

        virtual void inviteResponse(int guidId, bool response) {}

        virtual void leave(int guidId) {}

        virtual void kick(int guidId, int playerId) {}

        virtual void chat(int guidId, const std::string &text) {}

        virtual void memberList(int guidId) {}

        virtual void changeMemberPostion(int guildId, int playerId, int level) {}

        virtual void requestAlliance(int guildId, int otherGuildId) {}

        virtual void requestAllianceResponse(int guildId, int otherGuildId, bool response) {}

        virtual void endAlliance(int guildId, int otherGuildId) {}
};
}

#endif // GUILDHANDLER_H
