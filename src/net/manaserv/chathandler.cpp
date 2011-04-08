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

#include "net/manaserv/chathandler.h"

#include "actorspritemanager.h"
#include "being.h"
#include "client.h"
#include "channel.h"
#include "channelmanager.h"
#include "event.h"
#include "log.h"
#include "playerrelations.h"

#include "gui/widgets/channeltab.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/manaserv_protocol.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <string>
#include <iostream>

extern Being *player_node;

extern Net::ChatHandler *chatHandler;

namespace ManaServ {

extern Connection *chatServerConnection;
extern Connection *gameServerConnection;
extern std::string netToken;
extern ServerInfo chatServer;

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
        CPMSG_DISCONNECT_RESPONSE,
        0
    };
    handledMessages = _messages;
    chatHandler = this;
}

void ChatHandler::handleMessage(Net::MessageIn &msg)
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
        case CPMSG_DISCONNECT_RESPONSE:
        {
            int errMsg = msg.readInt8();
            // Successful logout
            if (errMsg == ERRMSG_OK)
            {
                // TODO: Handle logout
            }
            else
            {
                switch (errMsg)
                {
                    case ERRMSG_NO_LOGIN:
                        errorMessage = "Chatserver: Not logged in";
                        break;
                    default:
                        errorMessage = "Chatserver: Unknown error";
                        break;
                }
                Client::setState(STATE_ERROR);
            }
        }
            break;
    }
}

void ChatHandler::handleGameChatMessage(Net::MessageIn &msg)
{
    short id = msg.readInt16();
    std::string chatMsg = msg.readString();

    if (id == 0)
    {
        SERVER_NOTICE(chatMsg)
        return;
    }

    Being *being = actorSpriteManager->findBeing(id);

    if (!being)
    {
        logger->log("Warning: Received GPMSG_SAY for unknown being with id %i."
                    " (Message is: %s)", id, chatMsg.c_str());
        return;
    }

    std::string mes = being->getName() + " : " + chatMsg;

    Event event(being == player_node ? Event::Player
                                           : Event::Being);
    event.setString("message", mes);
    event.setString("text", chatMsg);
    event.setString("nick", being->getName());
    event.setInt("beingId", id);
    event.setInt("permissions", player_relations
                 .checkPermissionSilently(being->getName(),
                 PlayerRelation::SPEECH_LOG | PlayerRelation::SPEECH_FLOAT));
    event.trigger(Event::ChatChannel);
}

void ChatHandler::handleEnterChannelResponse(Net::MessageIn &msg)
{
    if (msg.readInt8() == ERRMSG_OK)
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
        tab->chatLog(_("Players in this channel:"), BY_CHANNEL);
        while (msg.getUnreadLength())
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
        SERVER_NOTICE(_("Error joining channel."))
    }
}

void ChatHandler::handleListChannelsResponse(Net::MessageIn &msg)
{
    SERVER_NOTICE(_("Listing channels."))
    while (msg.getUnreadLength())
    {
        std::string channelName = msg.readString();
        if (channelName == "")
            return;
        std::ostringstream numUsers;
        numUsers << msg.readInt16();
        channelName += " - ";
        channelName += numUsers.str();
        SERVER_NOTICE(channelName)
    }
    SERVER_NOTICE(_("End of channel list."))
}

void ChatHandler::handlePrivateMessage(Net::MessageIn &msg)
{
    std::string userNick = msg.readString();
    std::string chatMsg = msg.readString();

    Event event(Event::Whisper);
    event.setString("nick", userNick);
    event.setString("message", chatMsg);
    event.trigger(Event::ChatChannel);
}

void ChatHandler::handleAnnouncement(Net::MessageIn &msg)
{
    std::string chatMsg = msg.readString();
    Event event(Event::Announcement);
    event.setString("message", chatMsg);
    event.trigger(Event::ChatChannel);
}

void ChatHandler::handleChatMessage(Net::MessageIn &msg)
{
    short channelId = msg.readInt16();
    std::string userNick = msg.readString();
    std::string chatMsg = msg.readString();

    Channel *channel = channelManager->findById(channelId);
    channel->getTab()->chatLog(userNick, chatMsg);
}

void ChatHandler::handleQuitChannelResponse(Net::MessageIn &msg)
{
    if (msg.readInt8() == ERRMSG_OK)
    {
        short channelId = msg.readInt16();
        Channel *channel = channelManager->findById(channelId);
        channelManager->removeChannel(channel);
    }
}

void ChatHandler::handleListChannelUsersResponse(Net::MessageIn &msg)
{
    std::string channelName = msg.readString();
    std::string userNick;
    std::string userModes;
    Channel *channel = channelManager->findByName(channelName);
    channel->getTab()->chatLog(_("Players in this channel:"), BY_CHANNEL);
    while (msg.getUnreadLength())
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

void ChatHandler::handleChannelEvent(Net::MessageIn &msg)
{
    short channelId = msg.readInt16();
    char eventId = msg.readInt8();
    std::string line = msg.readString();
    Channel *channel = channelManager->findById(channelId);

    if (channel)
    {
        switch(eventId)
        {
            case CHAT_EVENT_NEW_PLAYER:
                channel->getTab()->chatLog(strprintf(_("%s entered the "
                        "channel."), line.c_str()), BY_CHANNEL);
                break;

            case CHAT_EVENT_LEAVING_PLAYER:
                channel->getTab()->chatLog(strprintf(_("%s left the channel."),
                        line.c_str()), BY_CHANNEL);
                break;

            case CHAT_EVENT_TOPIC_CHANGE:
                channel->getTab()->chatLog(strprintf(_("Topic: %s"),
                        line.c_str()), BY_CHANNEL);
                break;

            case CHAT_EVENT_MODE_CHANGE:
            {
                int first = line.find(":");
                int second = line.find(":", first+1);
                std::string user1 = line.substr(0, first);
                std::string user2 = line.substr(first+1, second);
                std::string mode = line.substr(second+1, line.length());
                channel->getTab()->chatLog(strprintf(_("%s has set mode %s "
                        "on user %s."), user1.c_str(), mode.c_str(),
                        user2.c_str()), BY_CHANNEL);
            } break;

            case CHAT_EVENT_KICKED_PLAYER:
            {
                int first = line.find(":");
                std::string user1 = line.substr(0, first);
                std::string user2 = line.substr(first+1, line.length());
                channel->getTab()->chatLog(strprintf(_("%s has kicked %s."),
                        user1.c_str(), user2.c_str()), BY_CHANNEL);
            } break;

            default:
                channel->getTab()->chatLog(_("Unknown channel event."),
                                           BY_CHANNEL);
        }
    }
}

void ChatHandler::handleWhoResponse(Net::MessageIn &msg)
{
    std::string userNick;

    while (msg.getUnreadLength())
    {
        userNick = msg.readString();
        if (userNick == "")
        {
            break;
        }
        SERVER_NOTICE(userNick)
    }
}

void ChatHandler::connect()
{
    MessageOut msg(PCMSG_CONNECT);
    msg.writeString(netToken, 32);
    chatServerConnection->send(msg);
}

bool ChatHandler::isConnected()
{
    return chatServerConnection->isConnected();
}

void ChatHandler::disconnect()
{
    chatServerConnection->disconnect();
}

void ChatHandler::talk(const std::string &text)
{
    MessageOut msg(PGMSG_SAY);
    msg.writeString(text);
    gameServerConnection->send(msg);
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
    chatServerConnection->send(msg);
}

void ChatHandler::channelList()
{
    MessageOut msg(PCMSG_LIST_CHANNELS);
    chatServerConnection->send(msg);
}

void ChatHandler::enterChannel(const std::string &channel,
                               const std::string &password)
{
    MessageOut msg(PCMSG_ENTER_CHANNEL);
    msg.writeString(channel);
    msg.writeString(password);
    chatServerConnection->send(msg);
}

void ChatHandler::quitChannel(int channelId)
{
    MessageOut msg(PCMSG_QUIT_CHANNEL);
    msg.writeInt16(channelId);
    chatServerConnection->send(msg);
}

void ChatHandler::sendToChannel(int channelId, const std::string &text)
{
    MessageOut msg(PCMSG_CHAT);
    msg.writeString(text);
    msg.writeInt16(channelId);
    chatServerConnection->send(msg);
}

void ChatHandler::userList(const std::string &channel)
{
    MessageOut msg(PCMSG_LIST_CHANNELUSERS);
    msg.writeString(channel);
    chatServerConnection->send(msg);
}

void ChatHandler::setChannelTopic(int channelId, const std::string &text)
{
    MessageOut msg(PCMSG_TOPIC_CHANGE);
    msg.writeInt16(channelId);
    msg.writeString(text);
    chatServerConnection->send(msg);
}

void ChatHandler::setUserMode(int channelId, const std::string &name, int mode)
{
    MessageOut msg(PCMSG_USER_MODE);
    msg.writeInt16(channelId);
    msg.writeString(name);
    msg.writeInt8(mode);
    chatServerConnection->send(msg);
}

void ChatHandler::kickUser(int channelId, const std::string &name)
{
    MessageOut msg(PCMSG_KICK_USER);
    msg.writeInt16(channelId);
    msg.writeString(name);
    chatServerConnection->send(msg);
}

void ChatHandler::who()
{
    MessageOut msg(PCMSG_WHO);
    chatServerConnection->send(msg);
}

} // namespace ManaServ
