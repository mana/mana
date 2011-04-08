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

#include "net/tmwa/chathandler.h"

#include "actorspritemanager.h"
#include "being.h"
#include "event.h"
#include "game.h"
#include "localplayer.h"
#include "playerrelations.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "net/tmwa/protocol.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <string>

extern Net::ChatHandler *chatHandler;

namespace TmwAthena {

ChatHandler::ChatHandler()
{
    static const Uint16 _messages[] = {
        SMSG_BEING_CHAT,
        SMSG_PLAYER_CHAT,
        SMSG_WHISPER,
        SMSG_WHISPER_RESPONSE,
        SMSG_GM_CHAT,
        SMSG_MVP, // MVP
        0
    };
    handledMessages = _messages;
    chatHandler = this;
}

void ChatHandler::handleMessage(Net::MessageIn &msg)
{
    Being *being;
    std::string chatMsg;
    std::string nick;
    int chatMsgLength;

    switch (msg.getId())
    {
        case SMSG_WHISPER_RESPONSE:
            if (mSentWhispers.empty())
                nick = "user";
            else
            {
                nick = mSentWhispers.front();
                mSentWhispers.pop();
            }

            switch (msg.readInt8())
            {
                case 0x00:
                    // Success (don't need to report)
                    break;
                case 0x01:
                    {
                        Event event(Event::WhisperError);
                        event.setString("nick", nick);
                        event.setString("error", strprintf(_("Whisper could "
                                  "not be sent, %s is offline."), nick.c_str()));
                        event.trigger(Event::ChatChannel);
                    }
                    break;
                case 0x02:
                    {
                        Event event(Event::WhisperError);
                        event.setString("nick", nick);
                        event.setString("error", strprintf(_("Whisper could "
                                 "not be sent, ignored by %s."), nick.c_str()));
                        event.Event::trigger(Event::ChatChannel);
                    }
                    break;
            }
            break;

        // Received whisper
        case SMSG_WHISPER:
            chatMsgLength = msg.readInt16() - 28;
            nick = msg.readString(24);

            if (chatMsgLength <= 0)
                break;

            chatMsg = msg.readString(chatMsgLength);

            if (nick != "Server")
            {
                if (player_relations.hasPermission(nick, PlayerRelation::WHISPER))
                {
                    Event event(Event::Whisper);
                    event.setString("nick", nick);
                    event.setString("message", chatMsg);
                    event.trigger(Event::ChatChannel);
                }
            }
            else
            {
                SERVER_NOTICE(chatMsg)
            }

            break;

        // Received speech from being
        case SMSG_BEING_CHAT:
        {
            chatMsgLength = msg.readInt16() - 8;
            int beingId = msg.readInt32();
            being = actorSpriteManager->findBeing(beingId);

            if (!being || chatMsgLength <= 0)
                break;

            chatMsg = msg.readString(chatMsgLength);

            std::string::size_type pos = chatMsg.find(" : ", 0);
            std::string sender_name = ((pos == std::string::npos)
                                       ? "" : chatMsg.substr(0, pos));

            if (sender_name != being->getName()
                && being->getType() == Being::PLAYER)
            {
                if (!being->getName().empty())
                    sender_name = being->getName();
            }
            else
            {
                chatMsg.erase(0, pos + 3);
            }

            int perms;

            if (being->getType() == Being::PLAYER)
            {
                perms = player_relations.checkPermissionSilently(sender_name,
                    PlayerRelation::SPEECH_LOG | PlayerRelation::SPEECH_FLOAT);
            }
            else
            {
                perms = player_relations.getDefault()
                        & (PlayerRelation::SPEECH_LOG
                           | PlayerRelation::SPEECH_FLOAT);
            }

            trim(chatMsg);

            std::string reducedMessage = chatMsg;
            chatMsg = removeColors(sender_name) + " : " + reducedMessage;

            Event event(Event::Being);
            event.setString("message", chatMsg);
            event.setString("text", reducedMessage);
            event.setString("nick", sender_name);
            event.setInt("beingId", beingId);
            event.setInt("permissions", perms);
            event.trigger(Event::ChatChannel);

            break;
        }

        case SMSG_PLAYER_CHAT:
        case SMSG_GM_CHAT:
        {
            chatMsgLength = msg.readInt16() - 4;

            if (chatMsgLength <= 0)
                break;

            chatMsg = msg.readString(chatMsgLength);

            if (msg.getId() == SMSG_PLAYER_CHAT)
            {
                std::string::size_type pos = chatMsg.find(" : ", 0);
                std::string mes = chatMsg;

                if (pos != std::string::npos)
                    chatMsg.erase(0, pos + 3);

                trim(chatMsg);

                Event event(Event::Player);
                event.setString("message", mes);
                event.setString("text", chatMsg);
                event.setString("nick", player_node->getName());
                event.setInt("beingId", player_node->getId());
                event.setInt("permissions", player_relations.getDefault()
                             & (PlayerRelation::SPEECH_LOG
                                | PlayerRelation::SPEECH_FLOAT));
                event.trigger(Event::ChatChannel);
            }
            else
            {
                Event event(Event::Announcement);
                event.setString("message", chatMsg);
                event.trigger(Event::ChatChannel);
            }
            break;
        }

        case SMSG_MVP:
            // Display MVP player
            msg.readInt32(); // id
            SERVER_NOTICE(_("MVP player."))
            break;
    }
}

void ChatHandler::talk(const std::string &text)
{
    std::string mes = player_node->getName() + " : " + text;

    MessageOut outMsg(CMSG_CHAT_MESSAGE);
    // Added + 1 in order to let eAthena parse admin commands correctly
    outMsg.writeInt16(mes.length() + 4 + 1);
    outMsg.writeString(mes, mes.length() + 1);
}

void ChatHandler::me(const std::string &text)
{
    std::string action = strprintf("*%s*", text.c_str());

    talk(action);
}

void ChatHandler::privateMessage(const std::string &recipient,
                                 const std::string &text)
{
    MessageOut outMsg(CMSG_CHAT_WHISPER);
    outMsg.writeInt16(text.length() + 28);
    outMsg.writeString(recipient, 24);
    outMsg.writeString(text, text.length());

    mSentWhispers.push(recipient);
}

void ChatHandler::channelList()
{
    SERVER_NOTICE(_("Channels are not supported!"))
}

void ChatHandler::enterChannel(const std::string &channel,
                               const std::string &password)
{
    SERVER_NOTICE(_("Channels are not supported!"))
}

void ChatHandler::quitChannel(int channelId)
{
    SERVER_NOTICE(_("Channels are not supported!"))
}

void ChatHandler::sendToChannel(int channelId, const std::string &text)
{
    SERVER_NOTICE(_("Channels are not supported!"))
}

void ChatHandler::userList(const std::string &channel)
{
    SERVER_NOTICE(_("Channels are not supported!"))
}

void ChatHandler::setChannelTopic(int channelId, const std::string &text)
{
    SERVER_NOTICE(_("Channels are not supported!"))
}

void ChatHandler::setUserMode(int channelId, const std::string &name, int mode)
{
    SERVER_NOTICE(_("Channels are not supported!"))
}

void ChatHandler::kickUser(int channelId, const std::string &name)
{
    SERVER_NOTICE(_("Channels are not supported!"))
}

void ChatHandler::who()
{
    MessageOut outMsg(CMSG_WHO_REQUEST);
}

} // namespace TmwAthena
