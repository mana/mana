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

#include "messagein.h"
#include "protocol.h"

#include "../being.h"
#include "../beingmanager.h"
#include "../game.h"

#include "../gui/chat.h"

#include "../utils/tostring.h"
#include "../utils/trim.h"

extern Being *player_node;

ChatHandler::ChatHandler()
{
    static const Uint16 _messages[] = {
        SMSG_BEING_CHAT,
        SMSG_PLAYER_CHAT,
        SMSG_WHISPER,
        SMSG_WHISPER_RESPONSE,
        SMSG_GM_CHAT,
        SMSG_WHO_ANSWER,
        0x10c, // MVP
        0
    };
    handledMessages = _messages;
}

void ChatHandler::handleMessage(MessageIn *msg)
{
    Being *being;
    std::string chatMsg;
    std::string nick;
    Sint16 chatMsgLength;

    switch (msg->getId())
    {
        case SMSG_WHISPER_RESPONSE:
            switch (msg->readInt8())
            {
                case 0x00:
                    // comment out since we'll local echo in chat.cpp instead, then only report failures
                    //chatWindow->chatLog("Whisper sent", BY_SERVER);
                    break;
                case 0x01:
                    chatWindow->chatLog("Whisper could not be sent, user is offline", BY_SERVER);
                    break;
                case 0x02:
                    chatWindow->chatLog("Whisper could not be sent, ignored by user", BY_SERVER);
                    break;
            }
            break;

        // Received whisper
        case SMSG_WHISPER:
            chatMsgLength = msg->readInt16() - 28;
            nick = msg->readString(24);

            if (chatMsgLength <= 0)
                break;

            chatMsg = msg->readString(chatMsgLength);
            if (nick != "Server")
                chatMsg = nick + " : " + chatMsg;
            chatWindow->chatLog(chatMsg, (nick == "Server") ? BY_SERVER : ACT_WHISPER);

            break;

        // Received speech from being
        case SMSG_BEING_CHAT:
            chatMsgLength = msg->readInt16() - 8;
            being = beingManager->findBeing(msg->readInt32());

            if (!being || chatMsgLength <= 0)
            {
                break;
            }

            chatMsg = msg->readString(chatMsgLength);
            chatWindow->chatLog(chatMsg, BY_OTHER);
            chatMsg.erase(0, chatMsg.find(" : ", 0) + 3);
            trim(chatMsg);
            being->setSpeech(chatMsg, SPEECH_TIME);
            break;

        case SMSG_PLAYER_CHAT:
        case SMSG_GM_CHAT:
            chatMsgLength = msg->readInt16() - 4;

            if (chatMsgLength <= 0)
            {
                break;
            }

            chatMsg = msg->readString(chatMsgLength);

            if (msg->getId() == SMSG_PLAYER_CHAT)
            {
                chatWindow->chatLog(chatMsg, BY_PLAYER);

                std::string::size_type pos = chatMsg.find(" : ", 0);
                if (pos != std::string::npos)
                {
                    chatMsg.erase(0, pos + 3);
                }
                trim(chatMsg);
                player_node->setSpeech(chatMsg, SPEECH_TIME);
            }
            else
            {
                chatWindow->chatLog(chatMsg, BY_GM);
            }
            break;

        case SMSG_WHO_ANSWER:
            chatWindow->chatLog("Online users: " + toString(msg->readInt32()),
                    BY_SERVER);
            break;

        case 0x010c:
            // Display MVP player
            msg->readInt32(); // id
            chatWindow->chatLog("MVP player", BY_SERVER);
            break;
    }
}
