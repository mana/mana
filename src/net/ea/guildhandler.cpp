/*
 *  The Mana World
 *  Copyright (C) 2009-2010  The Mana World Development Team
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

#include "net/ea/guildhandler.h"

#include "localplayer.h"
#include "log.h"

#include "gui/widgets/chattab.h"

#include "net/ea/messagein.h"
#include "net/ea/protocol.h"

#include "utils/gettext.h"

extern Net::GuildHandler *guildHandler;

namespace EAthena {

GuildHandler::GuildHandler()
{
    static const Uint16 _messages[] = {
        SMSG_GUILD_CREATE_RESPONSE,
        SMSG_GUILD_INVITE_ACK,
        SMSG_GUILD_INVITE,
        0
    };
    handledMessages = _messages;

    guildHandler = this;
}

void GuildHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_GUILD_CREATE_RESPONSE:
            {
                int flag = msg.readInt8();

                if (flag == 0)
                {
                    // Success
                }
                else if (flag == 1)
                {
                    // Already in a guild
                }
                else if (flag == 2)
                {
                    // Unable to make (likely name already in use)
                }
                else if (flag == 3)
                {
                    // Emperium check failed
                }
            }
            break;
        case SMSG_GUILD_INVITE_ACK:
            {
                int flag = msg.readInt8();

                if (flag == 0)
                {
                    // Fail (already in guild, busy, etc)
                }
                else if (flag == 1)
                {
                    // Rejected
                }
                else if (flag == 2)
                {
                    // Accepted
                }
                else if (flag == 3)
                {
                    // Guild full
                }
            }
            break;

        case SMSG_GUILD_INVITE:
            {
                int guildId = msg.readInt32();
                std::string guildName = msg.readString(24);

                printf("Guild invite for %d (%s)\n", guildId, guildName.c_str());

                // TODO
            }
            break;

        case SMSG_GUILD_POSITION_INFO:
            {
                int guildId =  msg.readInt32();
                int emblem =  msg.readInt32();
                int posMode =  msg.readInt32();
                msg.readInt32(); // Unused
                msg.readInt8(); // Unused
                std::string guildName = msg.readString(24);

                logger->log("Guild position info: %d %d %d %s\n", guildId,
                            emblem, posMode, guildName.c_str());
            }
            break;
    }
}

void GuildHandler::create(const std::string &name)
{
    localChatTab->chatLog(_("Guild creation isn't supported yet."),
                          BY_SERVER);
    return;

    MessageOut msg(CMSG_GUILD_CREATE);
    msg.writeInt32(0); // Unused
    msg.writeString(name, 24);
}

void GuildHandler::invite(int guildId, const std::string &name)
{
    // TODO
}

void GuildHandler::invite(int guildId, Player *player)
{
    MessageOut msg(CMSG_GUILD_INVITE);
    msg.writeInt32(player->getId());
    msg.writeInt32(0); // Unused
    msg.writeInt32(0); // Unused
}

void GuildHandler::inviteResponse(int guildId, bool response)
{
    MessageOut msg(CMSG_GUILD_INVITE_REPLY);
    msg.writeInt32(guildId);
    msg.writeInt8(response);
    msg.writeInt8(0); // Unused
    msg.writeInt16(0); // Unused
}

void GuildHandler::leave(int guildId)
{
    MessageOut msg(CMSG_GUILD_LEAVE);
    msg.writeInt32(guildId);
    msg.writeInt32(0); // Account ID
    msg.writeInt32(player_node->getId());
    msg.writeString("", 30); // Message
}

void GuildHandler::kick(GuildMember member)
{
    MessageOut msg(CMSG_GUILD_EXPULSION);
    msg.writeInt32(member.getGuild()->getId());
    msg.writeInt32(member.getID()); // Account ID
    msg.writeInt32(0); // Char ID
    msg.writeString("", 40); // Message
}

void GuildHandler::chat(int guildId, const std::string &text)
{
    MessageOut msg(CMSG_GUILD_MESSAGE);
    msg.writeInt16(text.size() + 4);
    msg.writeString(text);
}

void GuildHandler::memberList(int guildId)
{
    // TODO
}

void GuildHandler::changeMemberPostion(GuildMember member, int level)
{
    // TODO
}

void GuildHandler::requestAlliance(int guildId, int otherGuildId)
{
    // TODO
}

void GuildHandler::requestAllianceResponse(int guildId, int otherGuildId,
                             bool response)
{
    // TODO
}

void GuildHandler::endAlliance(int guildId, int otherGuildId)
{
    // TODO
}

}
