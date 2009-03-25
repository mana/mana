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

#include "chathandler.h"

#include <SDL_types.h>
#include <string>
#include <iostream>

#include "../messagein.h"
#include "protocol.h"

#include "../../being.h"
#include "../../beingmanager.h"
#include "../../game.h"
#include "../../channel.h"
#include "../../channelmanager.h"

#include "../../gui/chat.h"
#include "../../gui/guildwindow.h"

extern Being *player_node;

ChatHandler::ChatHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_SAY,
        CPMSG_ENTER_CHANNEL_RESPONSE,
        CPMSG_LIST_CHANNELS_RESPONSE,
        CPMSG_PUBMSG,
        CPMSG_ANNOUNCEMENT,
        CPMSG_PRIVMSG,
        CPMSG_QUIT_CHANNEL_RESPONSE,
        CPMSG_LIST_CHANNELUSERS_RESPONSE,
        CPMSG_CHANNEL_EVENT,
        0
    };
    handledMessages = _messages;
}

void ChatHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_SAY:
            handleGameChatMessage(msg);
            break;

        case CPMSG_ENTER_CHANNEL_RESPONSE:
            handleEnterChannelResponse(msg);
            break;

        case CPMSG_LIST_CHANNELS_RESPONSE:
            handleListChannelsResponse(msg);
            break;

        case CPMSG_PRIVMSG:
            handlePrivateMessage(msg);
            break;

        case CPMSG_ANNOUNCEMENT:
            handleAnnouncement(msg);
            break;

        case CPMSG_PUBMSG:
            handleChatMessage(msg);
            break;

        case CPMSG_QUIT_CHANNEL_RESPONSE:
            handleQuitChannelResponse(msg);
            break;

        case CPMSG_LIST_CHANNELUSERS_RESPONSE:
            handleListChannelUsersResponse(msg);
            break;

        case CPMSG_CHANNEL_EVENT:
            handleChannelEvent(msg);
    }
}

void ChatHandler::handleGameChatMessage(MessageIn &msg)
{
    short id = msg.readInt16();
    std::string chatMsg = msg.readString();

    if (id == 0)
    {
        chatWindow->chatLog(chatMsg, BY_SERVER);
        return;
    }

    Being *being = beingManager->findBeing(id);

    if (being)
    {
        chatWindow->chatLog(being->getName() + " : " + chatMsg,
                            being == player_node ? BY_PLAYER : BY_OTHER, "General");
        being->setSpeech(chatMsg, SPEECH_TIME);
    }
    else
    {
        chatWindow->chatLog("Unknown : " + chatMsg, BY_OTHER, "General");
    }
}

void ChatHandler::handleEnterChannelResponse(MessageIn &msg)
{
    if(msg.readInt8() == ERRMSG_OK)
    {
        short channelId = msg.readInt16();
        std::string channelName = msg.readString();
        std::string announcement = msg.readString();
        Channel *channel = new Channel(channelId, channelName, announcement);
        channelManager->addChannel(channel);
        chatWindow->createNewChannelTab(channelName);
        chatWindow->chatLog("Topic: " + announcement, BY_CHANNEL, channelName);

        std::string user;
        std::string userModes;
        chatWindow->chatLog("Players in this channel:", BY_CHANNEL, channelName);
        while(msg.getUnreadLength())
        {
            user = msg.readString();
            if (user == "")
                return;
            userModes = msg.readString();
            if (userModes.find('o') != std::string::npos)
            {
                user = "@" + user;
            }
            chatWindow->chatLog(user, BY_CHANNEL, channelName);
        }

    }
    else
    {
        chatWindow->chatLog("Error joining channel", BY_SERVER);
    }
}

void ChatHandler::handleListChannelsResponse(MessageIn &msg)
{
    chatWindow->chatLog("Listing Channels", BY_SERVER);
    while(msg.getUnreadLength())
    {
        std::string channelName = msg.readString();
        if (channelName == "")
            return;
        std::ostringstream numUsers;
        numUsers << msg.readInt16();
        channelName += " - ";
        channelName += numUsers.str();
        chatWindow->chatLog(channelName, BY_SERVER);
    }
    chatWindow->chatLog("End of channel list", BY_SERVER);
}

void ChatHandler::handlePrivateMessage(MessageIn &msg)
{
    std::string userNick = msg.readString();
    std::string chatMsg = msg.readString();

    if (!chatWindow->tabExists(userNick))
    {
        chatWindow->createNewChannelTab(userNick);

    }
    chatWindow->chatLog(userNick + ": " + chatMsg, BY_OTHER, userNick);
}

void ChatHandler::handleAnnouncement(MessageIn &msg)
{
    std::string chatMsg = msg.readString();
    chatWindow->chatLog(chatMsg, BY_GM);
}

void ChatHandler::handleChatMessage(MessageIn &msg)
{
    short channelId = msg.readInt16();
    std::string userNick = msg.readString();
    std::string chatMsg = msg.readString();

    chatWindow->sendToChannel(channelId, userNick, chatMsg);
}

void ChatHandler::handleQuitChannelResponse(MessageIn &msg)
{
    if(msg.readInt8() == ERRMSG_OK)
    {
        short channelId = msg.readInt16();
        // remove the chat tab
        chatWindow->removeChannel(channelId);
    }
}

void ChatHandler::handleListChannelUsersResponse(MessageIn &msg)
{
    std::string channel = msg.readString();
    std::string userNick;
    std::string userModes;
    chatWindow->chatLog("Players in this channel:", BY_CHANNEL, channel);
    while(msg.getUnreadLength())
    {
        userNick = msg.readString();
        if (userNick == "")
        {
            break;
        }
        userModes = msg.readString();
        if (userModes.find('o') != std::string::npos)
        {
            userNick = "@" + userNick;
        }
        chatWindow->chatLog(userNick, BY_CHANNEL, channel);
    }
}

void ChatHandler::handleChannelEvent(MessageIn &msg)
{
    short channelId = msg.readInt16();
    char eventId = msg.readInt8();
    std::string line = msg.readString();
    Channel *channel = channelManager->findById(channelId);

    if(channel)
    {
        switch(eventId)
        {
            case CHAT_EVENT_NEW_PLAYER:
                line += " entered the channel.";
                break;

            case CHAT_EVENT_LEAVING_PLAYER:
                line += " left the channel.";
                break;

            case CHAT_EVENT_TOPIC_CHANGE:
                line = "Topic: " + line;
                break;

            case CHAT_EVENT_MODE_CHANGE:
            {
                int first = line.find(":");
                int second = line.find(":", first+1);
                std::string user1 = line.substr(0, first);
                std::string user2 = line.substr(first+1, second);
                std::string mode = line.substr(second+1, line.length());
                line = user1 + " has set mode " + mode + " on user " + user2;
            } break;

            case CHAT_EVENT_KICKED_PLAYER:
            {
                int first = line.find(":");
                std::string user1 = line.substr(0, first);
                std::string user2 = line.substr(first+1, line.length());
                line = user1 + " has kicked " + user2;
            } break;

            default:
                line = "Unknown channel event.";
        }

        chatWindow->chatLog(line, BY_CHANNEL, channel->getName());
    }
}

