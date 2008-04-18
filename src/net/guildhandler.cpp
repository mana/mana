/*
 *  guildhandler.cpp
 *  A file part of The Mana World
 *
 * Copyright (c) 2007, The Mana World Development Team
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * My name may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 * $Id:$
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
