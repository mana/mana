/*
 *  The Mana World
 *  Copyright (C) 2008  Lloyd Bryant <lloyd_bryant@netzero.net>
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

#include "being.h"
#include "localplayer.h"
#include "party.h"

#include "gui/widgets/chattab.h"
#include "gui/chat.h"
#include "gui/confirm_dialog.h"
#include "gui/partywindow.h"

#include "net/messageout.h"
#include "net/ea/protocol.h"
#include "net/ea/gui/partytab.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/strprintf.h"

void eAthena::Party::create(const std::string &party)
{
    if (party.empty())
    {
        partyTab->chatLog(_("Party name is missing."), BY_SERVER);
        return;
    }
    MessageOut outMsg(CMSG_PARTY_CREATE);
    outMsg.writeString(party.substr(0, 23), 24);
}

void eAthena::Party::leave(const std::string &args)
{
    MessageOut outMsg(CMSG_PARTY_LEAVE);
    partyTab->chatLog(_("Left party."), BY_SERVER);
    player_node->setInParty(false);
}

void eAthena::Party::createResponse(bool ok)
{
    if (ok)
    {
        partyTab->chatLog(_("Party successfully created."), BY_SERVER);
        player_node->setInParty(true);
    }
    else
    {
        partyTab->chatLog(_("Could not create party."), BY_SERVER);
    }
}

void eAthena::Party::invite(Player *player)
{
    MessageOut outMsg(CMSG_PARTY_INVITE);
    outMsg.writeInt32(player->getId());
}

void eAthena::Party::respondToInvite(bool accept)
{
    MessageOut outMsg(CMSG_PARTY_INVITED);
    outMsg.writeInt32(player_node->getId());
    outMsg.writeInt32(accept ? 1 : 0);
    player_node->setInParty(player_node->getInParty() || accept);
}
