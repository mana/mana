/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "net/tmwa/adminhandler.h"

#include "actorspritemanager.h"
#include "being.h"
#include "event.h"
#include "playerrelations.h"

#include "net/chathandler.h"
#include "net/net.h"

#include "net/tmwa/messagein.h"
#include "net/tmwa/protocol.h"

#include "utils/gettext.h"

#include <string>

extern Net::AdminHandler *adminHandler;

namespace TmwAthena {

AdminHandler::AdminHandler()
{
    static const uint16_t _messages[] =
    {
        SMSG_ADMIN_KICK_ACK,
        SMSG_ADMIN_IP,
        0
    };
    handledMessages = _messages;
    adminHandler = this;
}

void AdminHandler::handleMessage(MessageIn &msg)
{
    int id;
    switch (msg.getId())
    {
        case SMSG_ADMIN_KICK_ACK:
            id = msg.readInt32();
            if (id == 0)
                SERVER_NOTICE(_("Kick failed!"))
            else
                SERVER_NOTICE(_("Kick succeeded!"))
            break;
        case SMSG_ADMIN_IP:
            id = msg.readInt32();
            int ip = msg.readInt32();
            if (Being *player = actorSpriteManager->findBeing(id))
            {
                player->setIp(ip);
                player->updateName();
            }
            break;
    }
}

void AdminHandler::kick(const std::string &name)
{
    Net::getChatHandler()->talk("@kick " + name);
}

void AdminHandler::ban(const std::string &name)
{
    Net::getChatHandler()->talk("@ban " + name);
}

void AdminHandler::unban(const std::string &name)
{
    Net::getChatHandler()->talk("@unban " + name);
}

} // namespace TmwAthena
