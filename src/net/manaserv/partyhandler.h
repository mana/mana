/*
 *  The Mana Client
 *  Copyright (C) 2008-2010  The Mana World Development Team
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

#ifndef NET_MANASERV_PARTYHANDLER_H
#define NET_MANASERV_PARTYHANDLER_H

#include "net/partyhandler.h"

#include "net/manaserv/messagehandler.h"

#include "party.h"

#include <string>

namespace ManaServ {

class PartyHandler : public MessageHandler, public Net::PartyHandler
{
public:
    PartyHandler();

    void handleMessage(Net::MessageIn &msg);

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
private:
    Party *mParty;
};

} // namespace ManaServ

#endif // NET_MANASERV_PARTYHANDLER_H
