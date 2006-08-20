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
        friend class MessageOut;

        /**
         * Constructor. Sets up the local host.
         */
        Network();

        /**
         * Destructor.
         */
        ~Network();

        typedef enum {
            ACCOUNT,
            GAME,
            CHAT
        } Server;

        /**
         * Connects to the given server with the specified address and port.
         * This method is non-blocking, use isConnected to check whether the
         * server is connected.
         */
        bool
        connect(Server server, const std::string &address, short port);

        /**
         * Disconnects from the given server.
         */
        void
        disconnect(Server server);

        /**
         * Registers a message handler. A message handler handles a certain
         * subset of incoming messages.
         */
        void
        registerHandler(MessageHandler *handler);

        /**
         * Unregisters a message handler.
         */
        void
        unregisterHandler(MessageHandler *handler);

        void
        clearHandlers();

        int
        getState() const { return mState; }

        /**
         * Returns whether the given server is connected.
         */
        bool
        isConnected(Server server) const;

        void
        flush();

        /**
         * Send a message to a given server. The server should be connected.
         */
        void
        send(Server server, const MessageOut &msg);

        enum State {
            NET_OK,
            NET_ERROR
        };

    private:
        /**
         * The local host.
         */
        ENetHost *mClient;

        /**
         * An array holding the peers of the account, game and chat servers.
         */
        ENetPeer *mServers[3];

        /**
         * Dispatches a message to the appropriate message handler and
         * destroys it afterwards.
         */
        void
        dispatchMessage(ENetPacket *packet);

        unsigned int mToSkip;

        int mState;

        typedef std::map<unsigned short, MessageHandler*> MessageHandlers;
        typedef MessageHandlers::iterator MessageHandlerIterator;
        MessageHandlers mMessageHandlers;
};

/** Convert an address from int format to string */
char *iptostring(int address);

// TODO: remove this global, just a temp solution.
extern Network *network;

#endif
