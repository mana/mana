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

#ifndef NET_EA_CHATHANDLER_H
#define NET_EA_CHATHANDLER_H

#include "net/messagehandler.h"
#include "net/net.h"

class ChatHandler : public MessageHandler, public Net::ChatHandler
{
    public:
        ChatHandler();

        virtual void handleMessage(MessageIn &msg);

        virtual void talk(const std::string &text);

        virtual void me(const std::string &text);

        virtual void privateMessage(const std::string &recipient,
                            const std::string &text);

        virtual void channelList();

        virtual void enterChannel(int channelId, const std::string &password);

        virtual void quitChannel(int channelId);

        virtual void sendToChannel(int channelId, const std::string &text);

        virtual void userList(int channelId);

        virtual void setChannelTopic(int channelId, const std::string &text);

        virtual void setUserMode(int channelId, const std::string &name, int mode);

        virtual void kickUser(int channelId, const std::string &name);
};

extern ChatHandler *chatHandler;

#endif // NET_EA_CHATHANDLER_H
