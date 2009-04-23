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

#ifndef NET_CHATSERVER_PARTY_H
#define NET_CHATSERVER_PARTY_H

#include <iosfwd>

namespace Net
{
    namespace ChatServer
    {
        namespace Party
        {
            /**
             * Invite a player to the party.
             */
            void invitePlayer(const std::string &name);

            /**
             * Accept an invite another player has sent to join their party
             */
            void acceptInvite(const std::string &name);

            /**
             * Reject an invite from another player to join their party
             */
            void rejectInvite(const std::string &name);

            /**
             * Get a list of party members
             */
            void getPartyMembers();

            /**
             * Leave party
             */
            void quitParty();
        }
    }
}

#endif
