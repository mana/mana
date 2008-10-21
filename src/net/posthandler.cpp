/*
 *  The Mana World
 *  Copyright 2009 The Mana World Development Team
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

#include "posthandler.h"

#include <string>
#include <sstream>

#include "messagein.h"
#include "protocol.h"

#include "../gui/chat.h"

PostHandler::PostHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_SEND_POST_RESPONSE,
        GPMSG_GET_POST_RESPONSE,
        0
    };
    handledMessages = _messages;
}

void PostHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_SEND_POST_RESPONSE:
        {
            if (msg.readInt8() == ERRMSG_OK)
            {
                chatWindow->chatLog("Post sent successfully");
            }
            else
            {
                chatWindow->chatLog("Unable to send post");
            }
        } break;

        case GPMSG_GET_POST_RESPONSE:
        {
            std::stringstream str;
            // get the name of the sender
            std::string sender = msg.readString();

            if (sender == "")
            {
                chatWindow->chatLog("No post found");
                break;
            }

            // get the message
            std::string letter = msg.readString();

            // put message together
            str << "Message from " << sender << " says: " << letter;
            chatWindow->chatLog(str.str());
        } break;
    }
}
