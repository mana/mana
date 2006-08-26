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

#ifndef _TMW_NETWORK_
#define _TMW_NETWORK_

#include <map>
#include <string>

#include <guichan.hpp>

#include <enet/enet.h>

class MessageHandler;
class MessageOut;

/**
 * The client network layer. Facilitates connecting and communicating to the
 * account, game and chat servers. Also routes incoming message to the
 * appropriate message handlers.
 */
class Network
{
    public:
        /**
         * Sets up the local host.
         */
        static void
        initialize();

        /**
         * Closes the connections.
         */
        static void
        finalize();

        enum Server {
            ACCOUNT,
            GAME,
            CHAT
        };

        enum State {
            NET_OK,
            NET_ERROR
        };

        /**
         * Connects to the given server with the specified address and port.
         * This method is non-blocking, use isConnected to check whether the
         * server is connected.
         */
        static bool
        connect(Server server, const std::string &address, short port);

        /**
         * Disconnects from the given server.
         */
        static void
        disconnect(Server server);

        /**
         * Registers a message handler. A message handler handles a certain
         * subset of incoming messages.
         */
        static void
        registerHandler(MessageHandler *handler);

        /**
         * Unregisters a message handler.
         */
        static void
        unregisterHandler(MessageHandler *handler);

        static void
        clearHandlers();

        static State
        getState();

        /**
         * Returns whether the given server is connected.
         */
        static bool
        isConnected(Server server);

        static void
        flush();

        /**
         * Sends a message to a given server. The server should be connected.
         */
        static void
        send(Server server, const MessageOut &msg);
};

/** Convert an address from int format to string */
char *iptostring(int address);

#endif
