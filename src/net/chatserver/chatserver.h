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

#ifndef _TMW_NET_CHATSERVER_CHATSERVER_H
#define _TMW_NET_CHATSERVER_CHATSERVER_H

#include <iosfwd>

namespace Net
{
    class Connection;

    namespace ChatServer
    {
        void connect(Net::Connection *connection, const std::string &token);

        void logout();

        void chat(short channel, const std::string &text);

        void announce(const std::string &text);

        void privMsg(const std::string &recipient, const std::string &text);

        void enterChannel(const std::string &channel, const std::string &password);

        void quitChannel(short channel);

        void getChannelList();

        void getUserList(const std::string &channel);

        void setChannelTopic(short channel, const std::string &topic);

        void setUserMode(short channel, const std::string &user, unsigned char mode);

        void kickUser(short channel, const std::string &user);

    }
}

#endif
