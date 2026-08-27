/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#pragma once

#include "utils/mutex.h"

#include "net/serverinfo.h"

#include "net/tmwa/messagehandler.h"
#include "net/tmwa/messageout.h"

#include <SDL_net.h>
#include <SDL_thread.h>

#include <map>
#include <memory>
#include <string>

/**
 * Protocol version, reported to the eAthena char and mapserver who can adjust
 * the protocol accordingly.
 */
#define CLIENT_PROTOCOL_VERSION      1
// 10 -> 11: SMSG_MAP_MASK DONE

namespace TmwAthena {

struct PacketInfo;

class Network
{
    public:
        Network();

        ~Network();

        bool connect(const ServerInfo &server);
        void disconnect();

        /**
         * Returns the server that was last passed to connect().
         */
        const ServerInfo &getServer() const;

        void registerHandler(MessageHandler *handler);
        void unregisterHandler(MessageHandler *handler);

        void clearHandlers();

        const char *messageName(uint16_t id) const;

        int getState() const { return mState; }

        const std::string &getError() const { return mError; }

        bool isConnected() const { return mState == CONNECTED; }

        void skip(int len);

        void dispatchMessages();

        void flush();

        // ERROR replaced by NET_ERROR because already defined in Windows
        enum {
            IDLE,
            CONNECTED,
            CONNECTING,
            DATA,
            NET_ERROR
        };

    private:
        friend int networkThread(void *data);
        friend class MessageOut;

        /**
         * Shared between a Network and its worker thread while connecting.
         * Since opening a connection can't be interrupted, the Network may
         * decide to abandon the thread by clearing the back-reference.
         */
        struct ConnectRequest
        {
            ConnectRequest(const ServerInfo &server, Network *network)
                : server(server)
                , network(network)
            {}

            const ServerInfo server;
            ThreadSafe<Network *> network;
        };

        void setError(const std::string &error);

        uint16_t readWord(int pos);

        static const PacketInfo *findPacketInfo(uint16_t id);

        void receive();

        TCPsocket mSocket = nullptr;

        char *mInBuffer, *mOutBuffer;
        unsigned int mInSize = 0;
        unsigned int mOutSize = 0;

        unsigned int mToSkip = 0;

        int mState = IDLE;
        std::string mError;

        SDL_Thread *mWorkerThread = nullptr;
        std::shared_ptr<ConnectRequest> mConnectRequest;
        Mutex mMutex;

        std::map<uint16_t, MessageHandler *> mMessageHandlers;
};

} // namespace TmwAthena
