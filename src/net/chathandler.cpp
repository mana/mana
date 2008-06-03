/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
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
 *  $Id$
 */

#include "chathandler.h"

#include <SDL_types.h>
#include <string>
#include <iostream>

#include "messagein.h"
#include "protocol.h"

#include "../being.h"
#include "../beingmanager.h"
#include "../game.h"
#include "../channel.h"
#include "../channelmanager.h"

#include "../gui/chat.h"
#include "../gui/guildwindow.h"

#include "../utils/tostring.h"

extern Being *player_node;

ChatHandler::ChatHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_SAY,
        CPMSG_REGISTER_CHANNEL_RESPONSE,
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
            
        case CPMSG_REGISTER_CHANNEL_RESPONSE:
            handleRegisterChannelResponse(msg);
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
                            being == player_node ? BY_PLAYER : BY_OTHER);
        being->setSpeech(chatMsg, SPEECH_TIME);
    }
    else
    {
        chatWindow->chatLog("Unknown : " + chatMsg, BY_OTHER);
    }
}

void ChatHandler::handleRegisterChannelResponse(MessageIn &msg)
{
    char error = msg.readInt8();
    if(error == ERRMSG_OK)
    {
        short channelId = msg.readInt16();
        std::string channelName = msg.readString();
        std::string channelAnnouncement = msg.readString();
        chatWindow->chatLog("Registered Channel " + channelName, BY_SERVER);
        channelManager->addChannel(new Channel(channelId, 
                                               channelName, 
                                               channelAnnouncement));
        chatWindow->createNewChannelTab(channelName);
    }
    else
    {
        if (error == ERRMSG_INVALID_ARGUMENT)
        {
            chatWindow->chatLog("Error registering channel - Invalid Channel Name given", BY_SERVER);
        }
        else
        {
            chatWindow->chatLog("Error registering channel", BY_SERVER);
        }
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
        std::string user;
        while(msg.getUnreadLength())
        {
            user = msg.readString();
            if (user == "")
                return;
            channel->addUser(user);
        }
        channelManager->addChannel(channel);
        chatWindow->createNewChannelTab(channelName);
        chatWindow->chatLog(announcement, BY_SERVER, channelName);
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
        if(channelName != "")
        {
            channelName += " - ";
            channelName += numUsers.str();
            chatWindow->chatLog(channelName, BY_SERVER);
        }
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
    std::string channelName = msg.readString();
    std::string userNick;
    while(msg.getUnreadLength())
    {
        userNick = msg.readString();
        if (userNick == "")
        {
            break;
        }
        guildWindow->setOnline(channelName, userNick, true);
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
        
            default:
                line = "Unknown channel event.";
        }
    
        chatWindow->chatLog(line, BY_SERVER, channel->getName());
    }
}

