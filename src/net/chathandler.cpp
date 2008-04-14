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

#include "../gui/chat.h"

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
        CPMSG_QUIT_CHANNEL_RESPONSE,
        /*
        SMSG_BEING_CHAT,
        SMSG_PLAYER_CHAT,
        SMSG_GM_CHAT,
        SMSG_WHO_ANSWER,
        0x10c, // MVP
        */
        0
    };
    handledMessages = _messages;
}

void ChatHandler::handleMessage(MessageIn &msg)
{
    Being *being;
    std::string chatMsg;
    short channelId;
    std::string userNick;
    std::string channelName;
    //Sint16 chatMsgLength;

    switch (msg.getId())
    {
        case GPMSG_SAY:
            being = beingManager->findBeing(msg.readInt16());
            chatMsg = msg.readString();
            if (being)
            {
                chatWindow->chatLog(being->getName() + " : " + chatMsg, being == player_node ? BY_PLAYER : BY_OTHER);
                being->setSpeech(chatMsg, SPEECH_TIME);
            }
            else
            {
                chatWindow->chatLog("John Doe : " + chatMsg, BY_OTHER);
            }
            break;
        case CPMSG_REGISTER_CHANNEL_RESPONSE:
            if(msg.readInt8() == ERRMSG_OK)
            {
                channelId = msg.readInt16();
                std::string channelName = msg.readString();
                chatWindow->chatLog("Registered Channel " + channelName, BY_SERVER);
                chatWindow->addChannel(channelId, channelName);
                chatWindow->createNewChannelTab(channelName);
            }
            else
            {
                chatWindow->chatLog("Error registering channel", BY_SERVER);
            }
            break;
        case CPMSG_ENTER_CHANNEL_RESPONSE:
            if(msg.readInt8() == ERRMSG_OK)
            {
                channelId = msg.readInt16();
                channelName = msg.readString();
                std::string announcement = msg.readString();
                std::vector<std::string> userList;
                while(msg.getUnreadLength())
                {
                    userList.push_back(msg.readString());
                }
                chatWindow->addChannel(channelId, channelName);
                chatWindow->createNewChannelTab(channelName);
                chatWindow->chatLog(announcement, BY_SERVER, channelName);
            }
            else
            {
                chatWindow->chatLog("Error joining channel", BY_SERVER);
            }
            break;

        case CPMSG_LIST_CHANNELS_RESPONSE:
            chatWindow->chatLog("Listing Channels", BY_SERVER);
            while(msg.getUnreadLength())
            {
                channelName = msg.readString();
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
            break;

        case CPMSG_PRIVMSG:
            userNick = msg.readString();
            chatMsg = msg.readString();

            if (!chatWindow->tabExists(userNick))
            {
                chatWindow->createNewChannelTab(userNick);
                chatWindow->chatLog(userNick + ": " + chatMsg, BY_OTHER, userNick);
            }
            break;

        case CPMSG_PUBMSG:
            channelId = msg.readInt16();
            userNick = msg.readString();
            chatMsg = msg.readString();

            chatWindow->sendToChannel(channelId, userNick, chatMsg);
            break;

        case CPMSG_QUIT_CHANNEL_RESPONSE:
            if(msg.readInt8() == ERRMSG_OK)
            {
                channelId = msg.readInt16();
                // remove the chat tab
                chatWindow->removeChannel(channelId);
            }
            break;
        /*
        // Received speech from being
        case SMSG_BEING_CHAT:
            chatMsgLength = msg.readInt16() - 8;
            being = beingManager->findBeing(msg.readInt32());

            if (!being || chatMsgLength <= 0)
            {
                break;
            }

            chatMsg = msg.readString(chatMsgLength);
            chatWindow->chatLog(chatMsg, BY_OTHER);
            chatMsg.erase(0, chatMsg.find(" : ", 0) + 3);
            trim(chatMsg);
            being->setSpeech(chatMsg, SPEECH_TIME);
            break;

        case SMSG_PLAYER_CHAT:
        case SMSG_GM_CHAT:
            chatMsgLength = msg.readInt16() - 4;

            if (chatMsgLength <= 0)
            {
                break;
            }

            chatMsg = msg.readString(chatMsgLength);

            if (msg.getId() == SMSG_PLAYER_CHAT)
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
            chatWindow->chatLog("Online users: " + toString(msg.readInt32()),
                    BY_SERVER);
            break;

        case 0x010c:
            // Display MVP player
            msg.readInt32(); // id
            chatWindow->chatLog("MVP player", BY_SERVER);
            break;
        */
    }
}
