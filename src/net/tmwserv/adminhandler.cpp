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

#include "net/tmwserv/adminhandler.h"

#include "net/tmwserv/chatserver/chatserver.h"

Net::AdminHandler *adminHandler;

namespace TmwServ {

AdminHandler::AdminHandler()
{
    adminHandler = this;
}

void AdminHandler::announce(const std::string &text)
{
    Net::ChatServer::announce(text);
}

void AdminHandler::localAnnounce(const std::string &text)
{
    // TODO
}

void AdminHandler::hide(bool hide)
{
    // TODO
}

void AdminHandler::kick(int playerId)
{
    // TODO
}

void AdminHandler::kick(const std::string &name)
{
    // TODO
}

void AdminHandler::ban(int playerId)
{
    // TODO
}

void AdminHandler::ban(const std::string &name)
{
    // TODO
}

void AdminHandler::unban(int playerId)
{
    // TODO
}

void AdminHandler::unban(const std::string &name)
{
    // TODO
}

void AdminHandler::mute(int playerId, int type, int limit)
{
    // TODO
}

} // namespace TmwServ
