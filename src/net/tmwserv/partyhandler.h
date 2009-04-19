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

#ifndef NET_TMWSERV_PARTYHANDLER_H
#define NET_TMWSERV_PARTYHANDLER_H

#include "net/messagehandler.h"
#include "net/partyhandler.h"

#include <string>

namespace TmwServ {

class PartyHandler : public MessageHandler, public Net::PartyHandler
{
public:
    PartyHandler();

    void handleMessage(MessageIn &msg);

    void create(const std::string &name = "");

    void join(int partyId);

    void invite(Player *player);

    void invite(const std::string &name);

    void inviteResponse(const std::string &inviter, bool accept);

    void leave();

    void kick(Player *player);

    void kick(const std::string &name);

    void chat(const std::string &text);

    void requestPartyMembers();

    PartyShare getShareExperience() { return PARTY_SHARE_NO; }

    void setShareExperience(PartyShare share) {}

    PartyShare getShareItems() { return PARTY_SHARE_NO; }

    void setShareItems(PartyShare share) {}
};

} // namespace TmwServ

#endif

