/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "net/manaserv/guildhandler.h"

#include "guild.h"
#include "log.h"
#include "localplayer.h"
#include "channel.h"
#include "channelmanager.h"

#include "gui/widgets/channeltab.h"
#include "gui/chat.h"
#include "gui/socialwindow.h"

#include "net/messagein.h"
#include "net/net.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/protocol.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <iostream>

extern Net::GuildHandler *guildHandler;

namespace ManaServ {

extern Connection *chatServerConnection;

GuildHandler::GuildHandler()
{
    static const Uint16 _messages[] = {
        CPMSG_GUILD_CREATE_RESPONSE,
        CPMSG_GUILD_INVITE_RESPONSE,
        CPMSG_GUILD_ACCEPT_RESPONSE,
        CPMSG_GUILD_GET_MEMBERS_RESPONSE,
        CPMSG_GUILD_UPDATE_LIST,
        CPMSG_GUILD_INVITED,
        CPMSG_GUILD_REJOIN,
        CPMSG_GUILD_QUIT_RESPONSE,
        0
    };
    handledMessages = _messages;

    guildHandler = this;
}

void GuildHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case CPMSG_GUILD_CREATE_RESPONSE:
        {
            logger->log("Received CPMSG_GUILD_CREATE_RESPONSE");
            if (msg.readInt8() == ERRMSG_OK)
            {
                // TODO - Acknowledge guild was created
                localChatTab->chatLog(_("Guild created."));
                joinedGuild(msg);
            }
            else
            {
                localChatTab->chatLog(_("Error creating guild."));
            }
        } break;

        case CPMSG_GUILD_INVITE_RESPONSE:
        {
            logger->log("Received CPMSG_GUILD_INVITE_RESPONSE");
            if (msg.readInt8() == ERRMSG_OK)
            {
                // TODO - Acknowledge invite was sent
                localChatTab->chatLog(_("Invite sent."));
            }
        } break;

        case CPMSG_GUILD_ACCEPT_RESPONSE:
        {
            logger->log("Received CPMSG_GUILD_ACCEPT_RESPONSE");
            if (msg.readInt8() == ERRMSG_OK)
            {
                // TODO - Acknowledge accepted into guild
                joinedGuild(msg);
            }
        } break;

        case CPMSG_GUILD_GET_MEMBERS_RESPONSE:
        {
            logger->log("Received CPMSG_GUILD_GET_MEMBERS_RESPONSE");
            if (msg.readInt8() == ERRMSG_OK)
            {
                std::string name;
                bool online;
                Guild *guild;
                GuildMember *member;

                short guildId = msg.readInt16();
                guild = player_node->getGuild(guildId);

                if (!guild)
                    return;

                guild->clearMembers();

                while (msg.getUnreadLength())
                {
                    name = msg.readString();
                    online = msg.readInt8();
                    if (name != "")
                    {
                        member = new GuildMember(guildId, name);
                        member->setOnline(online);
                        guild->addMember(member);
                    }
                }
            }
        } break;

        case CPMSG_GUILD_UPDATE_LIST:
        {
            logger->log("Received CPMSG_GUILD_UPDATE_LIST");
            short guildId = msg.readInt16();
            std::string name = msg.readString();
            char eventId = msg.readInt8();
            GuildMember *member;

            Guild *guild = player_node->getGuild(guildId);
            if (guild)
            {
                switch(eventId)
                {
                    case GUILD_EVENT_NEW_PLAYER:
                        member = new GuildMember(guildId, name);
                        member->setOnline(true);
                        guild->addMember(member);
                        break;

                    case GUILD_EVENT_LEAVING_PLAYER:
                        guild->removeMember(name);
                        break;

                    case GUILD_EVENT_ONLINE_PLAYER:
                        member = guild->getMember(name);
                        if (member)
                        {
                            member->setOnline(true);
                        }
                        break;

                    case GUILD_EVENT_OFFLINE_PLAYER:
                        member = guild->getMember(name);
                        if (member)
                        {
                            member->setOnline(false);
                        }
                        break;

                    default:
                        logger->log("Invalid guild event");
                }
            }
        } break;

        case CPMSG_GUILD_INVITED:
        {
            logger->log("Received CPMSG_GUILD_INVITED");
            std::string inviterName = msg.readString();
            std::string guildName = msg.readString();
            int guildId = msg.readInt16();

            // Open a dialog asking if the player accepts joining the guild.
            socialWindow->showGuildInvite(guildName, guildId, inviterName);
        } break;

        case CPMSG_GUILD_PROMOTE_MEMBER_RESPONSE:
        {
            logger->log("Received CPMSG_GUILD_PROMOTE_MEMBER_RESPONSE");

            if (msg.readInt8() == ERRMSG_OK)
            {
                // promotion succeeded
                localChatTab->chatLog(_("Member was promoted successfully."));
            }
            else
            {
                // promotion failed
                localChatTab->chatLog(_("Failed to promote member."));
            }
        }

        case CPMSG_GUILD_REJOIN:
        {
            logger->log("Received CPMSG_GUILD_REJOIN");

            joinedGuild(msg);
        } break;

        case CPMSG_GUILD_QUIT_RESPONSE:
        {
            logger->log("Received CPMSG_GUILD_QUIT_RESPONSE");

            if (msg.readInt8() == ERRMSG_OK)
            {
                // Must remove tab first, as it wont find the guild
                // name after its removed from the player
                int guildId = msg.readInt16();
                Guild *guild = player_node->getGuild(guildId);
                if (guild)
                {
                    Channel *channel = channelManager->findByName(guild->getName());
                    channelManager->removeChannel(channel);
                    player_node->removeGuild(guildId);
                }
            }
        } break;
    }
}

void GuildHandler::joinedGuild(Net::MessageIn &msg)
{
    std::string guildName = msg.readString();
    short guildId = msg.readInt16();
    short permissions = msg.readInt16();
    short channelId = msg.readInt16();
    std::string announcement = msg.readString();

    // Add guild to player
    Guild *guild = Guild::getGuild(guildId);
    guild->setName(guildName);
    guild->setRights(permissions);
    player_node->addGuild(guild);
    Net::getGuildHandler()->memberList(guildId);

    // Automatically create the guild channel
    // COMMENT: Should this go here??
    Channel *channel = new Channel(channelId, guildName, announcement);
    channelManager->addChannel(channel);
    channel->getTab()->chatLog(strprintf(_("Topic: %s"), announcement.c_str()),
                               BY_CHANNEL);
}

void GuildHandler::create(const std::string &name)
{
    MessageOut msg(PCMSG_GUILD_CREATE);
    msg.writeString(name);
    chatServerConnection->send(msg);
}

void GuildHandler::invite(int guildId, const std::string &name)
{
    MessageOut msg(PCMSG_GUILD_INVITE);
    msg.writeInt16(guildId);
    msg.writeString(name);
    chatServerConnection->send(msg);
}

void GuildHandler::invite(int guildId, Player *player)
{
    invite(guildId, player->getName());
}

void GuildHandler::inviteResponse(int guildId, bool response)
{
    /*MessageOut msg(PCMSG_GUILD_ACCEPT);
    msg.writeString(name);
    chatServerConnection->send(msg);*/
}

void GuildHandler::leave(int guildId)
{
    MessageOut msg(PCMSG_GUILD_QUIT);
    msg.writeInt16(guildId);
    chatServerConnection->send(msg);
}

void GuildHandler::kick(GuildMember member)
{
    // TODO
}

void GuildHandler::chat(int guildId, const std::string &text)
{
    // TODO
}

void GuildHandler::memberList(int guildId)
{
    MessageOut msg(PCMSG_GUILD_GET_MEMBERS);
    msg.writeInt16(guildId);
    chatServerConnection->send(msg);
}

void GuildHandler::changeMemberPostion(GuildMember member, int level)
{
    /*MessageOut msg(PCMSG_GUILD_PROMOTE_MEMBER);
    msg.writeInt16(guildId);
    msg.writeString(name);
    msg.writeInt8(level);
    chatServerConnection->send(msg);*/
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

} // namespace ManaServ
