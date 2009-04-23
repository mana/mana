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

#include "net/chathandler.h"
#include "net/messagehandler.h"
#include "net/net.h"

namespace EAthena {

class ChatHandler : public MessageHandler, public Net::ChatHandler
{
    public:
        ChatHandler();

        void handleMessage(MessageIn &msg);

        void talk(const std::string &text);

        void me(const std::string &text);

        void privateMessage(const std::string &recipient,
                            const std::string &text);

        void channelList();

        void enterChannel(const std::string &channel,
                          const std::string &password);

        void quitChannel(int channelId);

        void sendToChannel(int channelId, const std::string &text);

        void userList(const std::string &channel);

        void setChannelTopic(int channelId, const std::string &text);

        void setUserMode(int channelId, const std::string &name, int mode);

        void kickUser(int channelId, const std::string &name);

        void who();
};

} // namespace EAthena

#endif // NET_EA_CHATHANDLER_H
