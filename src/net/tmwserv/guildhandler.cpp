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

#include "net/tmwserv/guildhandler.h"

#include "net/messagein.h"

#include "net/tmwserv/chatserver/chatserver.h"
#include "net/tmwserv/chatserver/guild.h"
#include "net/tmwserv/protocol.h"

#include "gui/widgets/channeltab.h"
#include "gui/chat.h"
#include "gui/guildwindow.h"

#include "guild.h"
#include "log.h"
#include "localplayer.h"
#include "channel.h"
#include "channelmanager.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <iostream>

namespace TmwServ {

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

}

void GuildHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case CPMSG_GUILD_CREATE_RESPONSE:
        {
            logger->log("Received CPMSG_GUILD_CREATE_RESPONSE");
            if(msg.readInt8() == ERRMSG_OK)
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
            if(msg.readInt8() == ERRMSG_OK)
            {
                // TODO - Acknowledge invite was sent
                localChatTab->chatLog(_("Invite sent."));
            }
        } break;

        case CPMSG_GUILD_ACCEPT_RESPONSE:
        {
            logger->log("Received CPMSG_GUILD_ACCEPT_RESPONSE");
            if(msg.readInt8() == ERRMSG_OK)
            {
                // TODO - Acknowledge accepted into guild
                joinedGuild(msg);
            }
        } break;

        case CPMSG_GUILD_GET_MEMBERS_RESPONSE:
        {
            logger->log("Received CPMSG_GUILD_GET_MEMBERS_RESPONSE");
            if(msg.readInt8() == ERRMSG_OK)
            {
                std::string guildMember;
                bool online;
                std::string guildName;
                Guild *guild;

                short guildId = msg.readInt16();
                guild = player_node->getGuild(guildId);

                if (!guild)
                    return;

                guildName = guild->getName();

                while(msg.getUnreadLength())
                {
                    guildMember = msg.readString();
                    online = msg.readInt8();
                    if(guildMember != "")
                    {
                        guild->addMember(guildMember);
                        guildWindow->setOnline(guildName, guildMember, online);
                    }
                }

                guildWindow->updateTab();
            }
        } break;

        case CPMSG_GUILD_UPDATE_LIST:
        {
            logger->log("Received CPMSG_GUILD_UPDATE_LIST");
            short guildId = msg.readInt16();
            std::string guildMember = msg.readString();
            char eventId = msg.readInt8();

            Guild *guild = player_node->getGuild(guildId);
            if (guild)
            {
                switch(eventId)
                {
                    case GUILD_EVENT_NEW_PLAYER:
                        guild->addMember(guildMember);
                        guildWindow->setOnline(guild->getName(), guildMember,
                                               true);
                        break;

                    case GUILD_EVENT_LEAVING_PLAYER:
                        guild->removeMember(guildMember);
                        break;

                    case GUILD_EVENT_ONLINE_PLAYER:
                        guildWindow->setOnline(guild->getName(), guildMember,
                                               true);
                        break;

                    case GUILD_EVENT_OFFLINE_PLAYER:
                        guildWindow->setOnline(guild->getName(), guildMember,
                                               false);
                        break;

                    default:
                        logger->log("Invalid guild event");
                }
            }
            guildWindow->updateTab();


        } break;

        case CPMSG_GUILD_INVITED:
        {
            logger->log("Received CPMSG_GUILD_INVITED");
            std::string inviterName = msg.readString();
            std::string guildName = msg.readString();

            // Open a dialog asking if the player accepts joining the guild.
            guildWindow->openAcceptDialog(inviterName, guildName);
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
                    guildWindow->removeTab(guildId);
                    player_node->removeGuild(guildId);
                }
            }
        } break;
    }
}

void GuildHandler::joinedGuild(MessageIn &msg)
{
    std::string guildName = msg.readString();
    short guildId = msg.readInt16();
    short permissions = msg.readInt16();
    short channelId = msg.readInt16();
    std::string announcement = msg.readString();

    // Add guild to player and create new guild tab
    Guild *guild = player_node->addGuild(guildId, permissions);
    guild->setName(guildName);
    guildWindow->newGuildTab(guildName);
    guildWindow->requestMemberList(guildId);

    // Automatically create the guild channel
    // COMMENT: Should this go here??
    Channel *channel = new Channel(channelId, guildName, announcement);
    channelManager->addChannel(channel);
    channel->getTab()->chatLog(strprintf(_("Topic: %s"), announcement.c_str()),
                               BY_CHANNEL);
}

} // namespace TmwServ
