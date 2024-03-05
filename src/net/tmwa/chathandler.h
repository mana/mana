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

#ifndef NET_TA_CHATHANDLER_H
#define NET_TA_CHATHANDLER_H

#include "net/chathandler.h"
#include "net/net.h"

#include "net/tmwa/messagehandler.h"

#include <queue>

namespace TmwAthena {

class ChatHandler final : public MessageHandler, public Net::ChatHandler
{
    public:
        ChatHandler();

        void handleMessage(MessageIn &msg) override;

        void talk(const std::string &text) override;

        void me(const std::string &text) override;

        void privateMessage(const std::string &recipient,
                            const std::string &text) override;

        void channelList() override;

        void enterChannel(const std::string &channel,
                          const std::string &password) override;

        void quitChannel(int channelId) override;

        void sendToChannel(int channelId, const std::string &text) override;

        void userList(const std::string &channel) override;

        void setChannelTopic(int channelId, const std::string &text) override;

        void setUserMode(int channelId, const std::string &name, int mode) override;

        void kickUser(int channelId, const std::string &name) override;

        void who() override {}

        bool whoSupported() const override { return false; }

    private:
        std::queue<std::string> mSentWhispers;
};

} // namespace TmwAthena

#endif // NET_TA_CHATHANDLER_H
