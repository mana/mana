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

#include "net/tmwserv/chathandler.h"

#include "net/tmwserv/connection.h"
#include "net/tmwserv/protocol.h"

#include "net/tmwserv/chatserver/chatserver.h"
#include "net/tmwserv/chatserver/internal.h"

#include "net/tmwserv/gameserver/internal.h"
#include "net/tmwserv/gameserver/player.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "being.h"
#include "beingmanager.h"
#include "game.h"
#include "channel.h"
#include "channelmanager.h"

#include "gui/widgets/channeltab.h"
#include "gui/chat.h"
#include "gui/guildwindow.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <string>
#include <iostream>

extern Being *player_node;

Net::ChatHandler *chatHandler;

namespace TmwServ {

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
        CPMSG_WHO_RESPONSE,
        0
    };
    handledMessages = _messages;
    chatHandler = this;
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
            break;

        case CPMSG_WHO_RESPONSE:
            handleWhoResponse(msg);
            break;
    }
}

void ChatHandler::handleGameChatMessage(MessageIn &msg)
{
    short id = msg.readInt16();
    std::string chatMsg = msg.readString();

    if (id == 0)
    {
        localChatTab->chatLog(chatMsg, BY_SERVER);
        return;
    }

    Being *being = beingManager->findBeing(id);

    std::string mes;
    if (being)
    {
        mes = being->getName() + " : " + chatMsg;
        being->setSpeech(chatMsg, SPEECH_TIME);
    }
    else
        mes = "Unknown : " + chatMsg;

    localChatTab->chatLog(mes, being == player_node ? BY_PLAYER : BY_OTHER);
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
        ChatTab *tab = channel->getTab();
        tab->chatLog(strprintf(_("Topic: %s"), announcement.c_str()), BY_CHANNEL);

        std::string user;
        std::string userModes;
        tab->chatLog("Players in this channel:", BY_CHANNEL);
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
            tab->chatLog(user, BY_CHANNEL);
        }

    }
    else
    {
        localChatTab->chatLog("Error joining channel.", BY_SERVER);
    }
}

void ChatHandler::handleListChannelsResponse(MessageIn &msg)
{
    localChatTab->chatLog("Listing channels", BY_SERVER);
    while(msg.getUnreadLength())
    {
        std::string channelName = msg.readString();
        if (channelName == "")
            return;
        std::ostringstream numUsers;
        numUsers << msg.readInt16();
        channelName += " - ";
        channelName += numUsers.str();
        localChatTab->chatLog(channelName, BY_SERVER);
    }
    localChatTab->chatLog("End of channel list.", BY_SERVER);
}

void ChatHandler::handlePrivateMessage(MessageIn &msg)
{
    std::string userNick = msg.readString();
    std::string chatMsg = msg.readString();

    chatWindow->whisper(userNick, chatMsg);
}

void ChatHandler::handleAnnouncement(MessageIn &msg)
{
    std::string chatMsg = msg.readString();
    localChatTab->chatLog(chatMsg, BY_GM);
}

void ChatHandler::handleChatMessage(MessageIn &msg)
{
    short channelId = msg.readInt16();
    std::string userNick = msg.readString();
    std::string chatMsg = msg.readString();

    Channel *channel = channelManager->findById(channelId);
    channel->getTab()->chatLog(userNick, chatMsg);
}

void ChatHandler::handleQuitChannelResponse(MessageIn &msg)
{
    if(msg.readInt8() == ERRMSG_OK)
    {
        short channelId = msg.readInt16();
        Channel *channel = channelManager->findById(channelId);
        channelManager->removeChannel(channel);
    }
}

void ChatHandler::handleListChannelUsersResponse(MessageIn &msg)
{
    std::string channelName = msg.readString();
    std::string userNick;
    std::string userModes;
    Channel *channel = channelManager->findByName(channelName);
    channel->getTab()->chatLog("Players in this channel:", BY_CHANNEL);
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
        localChatTab->chatLog(userNick, BY_CHANNEL, channel);
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

        channel->getTab()->chatLog(line, BY_CHANNEL);
    }
}

void ChatHandler::handleWhoResponse(MessageIn &msg)
{
    std::string userNick;

    while(msg.getUnreadLength())
    {
        userNick = msg.readString();
        if (userNick == "")
        {
            break;
        }
        localChatTab->chatLog(userNick, BY_SERVER);
    }
}

void ChatHandler::talk(const std::string &text)
{
    MessageOut msg(PGMSG_SAY);
    msg.writeString(text);
    Net::GameServer::connection->send(msg);
}

void ChatHandler::me(const std::string &text)
{
    // TODO
}

void ChatHandler::privateMessage(const std::string &recipient,
                                 const std::string &text)
{
    MessageOut msg(PCMSG_PRIVMSG);
    msg.writeString(recipient);
    msg.writeString(text);
    Net::ChatServer::connection->send(msg);
}

void ChatHandler::channelList()
{
    MessageOut msg(PCMSG_LIST_CHANNELS);
    Net::ChatServer::connection->send(msg);
}

void ChatHandler::enterChannel(const std::string &channel,
                               const std::string &password)
{
    MessageOut msg(PCMSG_ENTER_CHANNEL);
    msg.writeString(channel);
    msg.writeString(password);
    Net::ChatServer::connection->send(msg);
}

void ChatHandler::quitChannel(int channelId)
{
    MessageOut msg(PCMSG_QUIT_CHANNEL);
    msg.writeInt16(channelId);
    Net::ChatServer::connection->send(msg);
}

void ChatHandler::sendToChannel(int channelId, const std::string &text)
{
    MessageOut msg(PCMSG_CHAT);
    msg.writeString(text);
    msg.writeInt16(channelId);
    Net::ChatServer::connection->send(msg);
}

void ChatHandler::userList(const std::string &channel)
{
    MessageOut msg(PCMSG_LIST_CHANNELUSERS);
    msg.writeString(channel);
    Net::ChatServer::connection->send(msg);
}

void ChatHandler::setChannelTopic(int channelId, const std::string &text)
{
    MessageOut msg(PCMSG_TOPIC_CHANGE);
    msg.writeInt16(channelId);
    msg.writeString(text);
    Net::ChatServer::connection->send(msg);
}

void ChatHandler::setUserMode(int channelId, const std::string &name, int mode)
{
    MessageOut msg(PCMSG_USER_MODE);
    msg.writeInt16(channelId);
    msg.writeString(name);
    msg.writeInt8(mode);
    Net::ChatServer::connection->send(msg);
}

void ChatHandler::kickUser(int channelId, const std::string &name)
{
    MessageOut msg(PCMSG_KICK_USER);
    msg.writeInt16(channelId);
    msg.writeString(name);
    Net::ChatServer::connection->send(msg);
}

void ChatHandler::who()
{
    MessageOut msg(PCMSG_WHO);
    Net::ChatServer::connection->send(msg);
}

} // namespace TmwServ
