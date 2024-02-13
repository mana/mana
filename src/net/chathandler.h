/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef CHATHANDLER_H
#define CHATHANDLER_H

#include <string>

namespace Net {

class ChatHandler
{
    public:
        virtual ~ChatHandler() = default;

        virtual void talk(const std::string &text) = 0;

        virtual void me(const std::string &text) = 0;

        virtual void privateMessage(const std::string &recipient,
                            const std::string &text) = 0;

        virtual void channelList() = 0;

        virtual void enterChannel(const std::string &channel,
                                  const std::string &password) = 0;

        virtual void quitChannel(int channelId) = 0;

        virtual void sendToChannel(int channelId, const std::string &text) = 0;

        virtual void userList(const std::string &channel) = 0;

        virtual void setChannelTopic(int channelId, const std::string &text) = 0;

        virtual void setUserMode(int channelId, const std::string &name, int mode) = 0;

        virtual void kickUser(int channelId, const std::string &name) = 0;

        virtual void who() = 0;

        virtual bool whoSupported() const = 0;
};
}

#endif // CHATHANDLER_H
