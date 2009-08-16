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

#include "net/tmwserv/specialhandler.h"

#include "net/tmwserv/gameserver/internal.h"

#include "net/tmwserv/connection.h"
#include "net/tmwserv/protocol.h"

#include "net/messageout.h"

Net::SpecialHandler *specialHandler;

namespace TmwServ {

SpecialHandler::SpecialHandler()
{
    specialHandler = this;
}

void SpecialHandler::handleMessage(MessageIn &msg)
{
    // TODO
}

void SpecialHandler::use(int id)
{
    MessageOut msg(PGMSG_USE_SPECIAL);
    msg.writeInt8(id);
    Net::GameServer::connection->send(msg);
}

void SpecialHandler::use(int id, int level, int beingId)
{
    // TODO
}

void SpecialHandler::use(int id, int level, int x, int y)
{
    // TODO
}

void SpecialHandler::use(int id, const std::string &map)
{
    // TODO
}

} // namespace TmwServ
