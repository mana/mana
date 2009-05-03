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

#include "net/ea/adminhandler.h"

#include "net/ea/chathandler.h"
#include "net/ea/protocol.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "being.h"
#include "beingmanager.h"
#include "game.h"
#include "playerrelations.h"

#include "gui/widgets/chattab.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <string>

extern Net::ChatHandler *chatHandler;

Net::AdminHandler *adminHandler;

namespace EAthena {

AdminHandler::AdminHandler()
{
    static const Uint16 _messages[] = {
        SMSG_ADMIN_KICK_ACK,
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
                localChatTab->chatLog(_("Kick failed!"), BY_SERVER);
            else
                localChatTab->chatLog(_("Kick succeeded!"), BY_SERVER);
            break;
    }
}

void AdminHandler::announce(const std::string &text)
{
    MessageOut outMsg(CMSG_ADMIN_ANNOUNCE);
    outMsg.writeInt16(text.length() + 4);
    outMsg.writeString(text, text.length());
}

void AdminHandler::localAnnounce(const std::string &text)
{
    MessageOut outMsg(CMSG_ADMIN_LOCAL_ANNOUNCE);
    outMsg.writeInt16(text.length() + 4);
    outMsg.writeString(text, text.length());
}

void AdminHandler::hide(bool hide)
{
    MessageOut outMsg(CMSG_ADMIN_HIDE);
    outMsg.writeInt32(0); //unused
}

void AdminHandler::kick(int playerId)
{
    MessageOut outMsg(CMSG_ADMIN_KICK);
    outMsg.writeInt32(playerId);
}

void AdminHandler::kick(const std::string &name)
{
    chatHandler->talk("@kick " + name);
}

void AdminHandler::ban(int playerId)
{
    // Not supported
}

void AdminHandler::ban(const std::string &name)
{
    chatHandler->talk("@ban " + name);
}

void AdminHandler::unban(int playerId)
{
    // Not supported
}

void AdminHandler::unban(const std::string &name)
{
    chatHandler->talk("@unban " + name);
}

void AdminHandler::mute(int playerId, int type, int limit)
{
    return; // Still looking into this

    MessageOut outMsg(CMSG_ADMIN_MUTE);
    outMsg.writeInt32(playerId);
    outMsg.writeInt8(type);
    outMsg.writeInt16(limit);
}

} // namespace EAthena
