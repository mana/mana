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

#ifndef NET_TMWSERV_CONNECTION_H
#define NET_TMWSERV_CONNECTION_H

#include <iosfwd>

#include <enet/enet.h>

class MessageOut;

namespace Net
{
    /**
     * \ingroup Network
     */
    class Connection
    {
        public:
            enum State {
                OK,
                NET_ERROR
            };

            ~Connection();

            /**
             * Connects to the given server with the specified address and port.
             * This method is non-blocking, use isConnected to check whether the
             * server is connected.
             */
            bool connect(const std::string &address, short port);

            /**
             * Disconnects from the given server.
             */
            void disconnect();

            State getState() { return mState; }

            /**
             * Returns whether the server is connected.
             */
            bool isConnected();

            /**
             * Sends a message.
             */
            void send(const MessageOut &msg);

        private:
            friend Connection *Net::getConnection();
            Connection(ENetHost *client);

            ENetPeer *mConnection;
            ENetHost *mClient;
            State mState;
    };
}

#endif
