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
#include <queue>
#include <string>

#include <guichan.hpp>

#include <enet/enet.h>

class MessageHandler;
class MessageIn;
class MessageOut;

class Network
{
    public:
        friend class MessageOut;

        Network();
        ~Network();

        typedef enum {
            ACCOUNT,
            GAME,
            CHAT
        } Server;

        bool
        connect(Server server, const std::string &address, short port);

        void
        disconnect(Server server);

        void
        registerHandler(MessageHandler *handler);

        void
        unregisterHandler(MessageHandler *handler);

        void
        clearHandlers();

        int
        getState() const { return mState; }

        bool
        isConnected(Server server) const;

        void
        dispatchMessage(ENetPacket *packet);

        void
        flush();

        void
        send(Server server, const MessageOut &msg);

        enum State {
            NET_OK,
            NET_ERROR
        };

    private:
        ENetHost *mClient;

        ENetPeer *mAccountServer;
        ENetPeer *mGameServer;
        ENetPeer *mChatServer;

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
