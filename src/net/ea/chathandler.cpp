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

#include "net/ea/chathandler.h"

#include "net/ea/protocol.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "being.h"
#include "beingmanager.h"
#include "game.h"
#include "player_relations.h"

#include "gui/widgets/chattab.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/strprintf.h"

#include <string>

#define SERVER_NAME "Server"

ChatHandler *chatHandler;

ChatHandler::ChatHandler()
{
    static const Uint16 _messages[] = {
        SMSG_BEING_CHAT,
        SMSG_PLAYER_CHAT,
        SMSG_WHISPER,
        SMSG_WHISPER_RESPONSE,
        SMSG_GM_CHAT,
        0x10c, // MVP
        0
    };
    handledMessages = _messages;
    chatHandler = this;
}

void ChatHandler::handleMessage(MessageIn &msg)
{
    Being *being;
    std::string chatMsg;
    std::string nick;
    int chatMsgLength;

    switch (msg.getId())
    {
        case SMSG_WHISPER_RESPONSE:
            switch (msg.readInt8())
            {
                case 0x00:
                    // comment out since we'll local echo in chat.cpp instead, then only report failures
                    //localChatTab->chatLog("Whisper sent", BY_SERVER);
                    break;
                case 0x01:
                    localChatTab->chatLog(_("Whisper could not be sent, user is offline"), BY_SERVER);
                    break;
                case 0x02:
                    localChatTab->chatLog(_("Whisper could not be sent, ignored by user"), BY_SERVER);
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

            if (nick != SERVER_NAME)
            {
                if (player_relations.hasPermission(nick, PlayerRelation::WHISPER))
                    chatWindow->whisper(nick, chatMsg);
            }
            else
            {
                localChatTab->chatLog(chatMsg, BY_SERVER);
            }

            break;

        // Received speech from being
        case SMSG_BEING_CHAT: {
            chatMsgLength = msg.readInt16() - 8;
            being = beingManager->findBeing(msg.readInt32());

            if (!being || chatMsgLength <= 0)
                break;

            chatMsg = msg.readString(chatMsgLength);

            std::string::size_type pos = chatMsg.find(" : ", 0);
            std::string sender_name = ((pos == std::string::npos)
                                       ? ""
                                       : chatMsg.substr(0, pos));

            // We use getIgnorePlayer instead of ignoringPlayer here because ignorePlayer' side
            // effects are triggered right below for Being::IGNORE_SPEECH_FLOAT.
            if (player_relations.checkPermissionSilently(sender_name, PlayerRelation::SPEECH_LOG))
                localChatTab->chatLog(chatMsg, BY_OTHER);

            chatMsg.erase(0, pos + 3);
            trim(chatMsg);

            if (player_relations.hasPermission(sender_name, PlayerRelation::SPEECH_FLOAT))
                being->setSpeech(chatMsg, SPEECH_TIME);
            break;
        }

        case SMSG_PLAYER_CHAT:
        case SMSG_GM_CHAT: {
            chatMsgLength = msg.readInt16() - 4;

            if (chatMsgLength <= 0)
                break;

            chatMsg = msg.readString(chatMsgLength);
            std::string::size_type pos = chatMsg.find(" : ", 0);

            if (msg.getId() == SMSG_PLAYER_CHAT)
            {
                localChatTab->chatLog(chatMsg, BY_PLAYER);

                if (pos != std::string::npos)
                    chatMsg.erase(0, pos + 3);

                trim(chatMsg);

                player_node->setSpeech(chatMsg, SPEECH_TIME);
            }
            else
            {
                localChatTab->chatLog(chatMsg, BY_GM);
            }
            break;
        }

        case 0x010c:
            // Display MVP player
            msg.readInt32(); // id
            localChatTab->chatLog("MVP player", BY_SERVER);
            break;
    }
}

void ChatHandler::talk(const std::string &text)
{
    std::string mes = player_node->getName() + " : " + text;

    MessageOut outMsg(CMSG_CHAT_MESSAGE);
    // Added + 1 in order to let eAthena parse admin commands correctly
    outMsg.writeInt16(text.length() + 4 + 1);
    outMsg.writeString(text, text.length() + 1);
}

void ChatHandler::me(const std::string &text)
{
    std::string action = strprintf("*%s*", text.c_str());

    talk(text);
}

void ChatHandler::privateMessage(const std::string &recipient,
                                 const std::string &text)
{
    MessageOut outMsg(CMSG_CHAT_WHISPER);
    outMsg.writeInt16(text.length() + 28);
    outMsg.writeString(recipient, 24);
    outMsg.writeString(text, text.length());
}

void ChatHandler::channelList()
{
    // TODO
}

void ChatHandler::enterChannel(int channelId, const std::string &password)
{
    // TODO
}

void ChatHandler::quitChannel(int channelId)
{
    // TODO
}

void ChatHandler::sendToChannel(int channelId, const std::string &text)
{
    // TODO
}

void ChatHandler::userList(int channelId)
{
    // TODO
}

void ChatHandler::setChannelTopic(int channelId, const std::string &text)
{
    // TODO
}

void ChatHandler::setUserMode(int channelId, const std::string &name, int mode)
{
    // TODO
}

void ChatHandler::kickUser(int channelId, const std::string &name)
{
    // TODO
}
