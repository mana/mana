/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "being.h"
#include "beingmanager.h"
#include "game.h"
#include "player.h"
#include "playerrelations.h"

#include "gui/widgets/chattab.h"

#include "net/chathandler.h"
#include "net/net.h"

#include "net/tmwa/protocol.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <string>

extern Net::AdminHandler *adminHandler;

namespace TmwAthena {

AdminHandler::AdminHandler()
{
    static const Uint16 _messages[] = {
        SMSG_ADMIN_KICK_ACK,
        SMSG_ADMIN_IP,
        0
    };
    handledMessages = _messages;
    adminHandler = this;
}

void AdminHandler::handleMessage(Net::MessageIn &msg)
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
        case SMSG_ADMIN_IP:
            id = msg.readInt32();
            int ip = msg.readInt32();
            Player *player = (Player *)beingManager->findBeing(id);
            player->setIp(ip);
            player->updateName();
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
    Net::getChatHandler()->talk("@kick " + name);
}

void AdminHandler::ban(int playerId)
{
    // Not supported
}

void AdminHandler::ban(const std::string &name)
{
    Net::getChatHandler()->talk("@ban " + name);
}

void AdminHandler::unban(int playerId)
{
    // Not supported
}

void AdminHandler::unban(const std::string &name)
{
    Net::getChatHandler()->talk("@unban " + name);
}

void AdminHandler::mute(int playerId, int type, int limit)
{
    return; // Still looking into this

    MessageOut outMsg(CMSG_ADMIN_MUTE);
    outMsg.writeInt32(playerId);
    outMsg.writeInt8(type);
    outMsg.writeInt16(limit);
}

} // namespace TmwAthena
