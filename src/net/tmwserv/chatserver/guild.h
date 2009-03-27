/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#ifndef NET_CHATSERVER_GUILD_H
#define NET_CHATSERVER_GUILD_H

#include <iosfwd>

namespace Net
{
    namespace ChatServer
    {
        namespace Guild
        {
            /**
             * Create guild.
             */
            void createGuild(const std::string &name);

            /**
             * Invite a player to your guild.
             */
            void invitePlayer(const std::string &name, short guildId);

            /**
             * Accept an invite another player has sent to join their guild.
             */
            void acceptInvite(const std::string &name);

            /**
             * Get a list of members in a guild.
             */
            void getGuildMembers(short guildId);

            /**
             * Promote guild member
             */
            void promoteMember(const std::string &name, short guildId,
                               short level);

            /**
             * Quit guild.
             */
            void quitGuild(short guildId);
        }
    }
}

#endif
