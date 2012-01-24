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

#ifndef NET_MANASERV_CONNECTION_H
#define NET_MANASERV_CONNECTION_H

#include <enet/enet.h>

#include <iosfwd>

namespace ManaServ
{
    class MessageOut;

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
            void send(const ManaServ::MessageOut &msg);

        private:
            friend Connection *ManaServ::getConnection();
            Connection(ENetHost *client);

            short mPort;
            ENetPeer *mConnection;
            ENetHost *mClient;
            State mState;
    };
}

#endif // NET_MANASERV_CONNECTION_H
