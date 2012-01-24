/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#ifndef NET_MANASERV_CHATHANDLER_H
#define NET_MANASERV_CHATHANDLER_H

#include "net/chathandler.h"
#include "net/serverinfo.h"

#include "net/manaserv/messagehandler.h"

namespace ManaServ {

class ChatHandler : public MessageHandler, public Net::ChatHandler
{
    public:
        ChatHandler();

        /**
         * Handle the given message appropriately.
         */
        void handleMessage(Net::MessageIn &msg);

        void connect();

        bool isConnected();

        void disconnect();

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

    private:
        /**
         * Handle chat messages sent from the game server.
         */
        void handleGameChatMessage(Net::MessageIn &msg);

        /**
         * Handle channel entry responses.
         */
        void handleEnterChannelResponse(Net::MessageIn &msg);

        /**
         * Handle list channels responses.
         */
        void handleListChannelsResponse(Net::MessageIn &msg);

        /**
         * Handle private messages.
         */
        void handlePrivateMessage(Net::MessageIn &msg);

        /**
         * Handle announcements.
         */
        void handleAnnouncement(Net::MessageIn &msg);

        /**
         * Handle chat messages.
         */
        void handleChatMessage(Net::MessageIn &msg);

        /**
         * Handle quit channel responses.
         */
        void handleQuitChannelResponse(Net::MessageIn &msg);

        /**
         * Handle list channel users responses.
         */
        void handleListChannelUsersResponse(Net::MessageIn &msg);

        /**
         * Handle channel events.
         */
        void handleChannelEvent(Net::MessageIn &msg);

        /**
         * Handle who responses.
         */
        void handleWhoResponse(Net::MessageIn &msg);
};

} // namespace ManaServ

#endif
