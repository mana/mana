/*
 *  The Mana World
 *  Copyright 2008 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id: $
 */

#include <iostream>
#include "guildhandler.h"

#include "protocol.h"
#include "messagein.h"

#include "chatserver/chatserver.h"
#include "chatserver/guild.h"

#include "../gui/guildwindow.h"
#include "../gui/chat.h"
#include "../guild.h"
#include "../log.h"
#include "../localplayer.h"

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
                joinedGuild(msg);
            }
        } break;

        case CPMSG_GUILD_INVITE_RESPONSE:
        {
            logger->log("Received CPMSG_GUILD_INVITE_RESPONSE");
            if(msg.readInt8() == ERRMSG_OK)
            {
                // TODO - Acknowledge invite was sent
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
                    if(guildMember != "")
                    {
                        guild->addMember(guildMember);
                        guildWindow->setOnline(guildName, guildMember, false);
                    }
                }

                guildWindow->updateTab();

                Net::ChatServer::getUserList(guildName);
            }
        } break;

        case CPMSG_GUILD_UPDATE_LIST:
        {
            logger->log("Received CPMSG_GUILD_UPDATE_LIST");
            short guildId = msg.readInt16();
            std::string guildMember = msg.readString();

            Guild *guild = player_node->getGuild(guildId);
            if (guild)
            {
                guild->addMember(guildMember);
                guildWindow->setOnline(guild->getName(), guildMember, false);
                Net::ChatServer::getUserList(guild->getName());
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
                    chatWindow->removeChannel(guild->getName());
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
    bool leader = msg.readInt8();
    short channelId = msg.readInt16();

    // Add guild to player and create new guild tab
    Guild *guild = player_node->addGuild(guildId, leader);
    guild->setName(guildName);
    guildWindow->newGuildTab(guildName);
    guildWindow->requestMemberList(guildId);

    // Automatically create the guild channel
    // COMMENT: Should this go here??
    chatWindow->addChannel(channelId, guildName);
    chatWindow->createNewChannelTab(guildName);
    chatWindow->chatLog("Guild Channel", BY_SERVER, guildName);
}
