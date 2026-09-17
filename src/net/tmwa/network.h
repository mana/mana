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

#ifdef __EMSCRIPTEN__
#include <emscripten/websocket.h>
#else
#include <SDL_net.h>
#include <SDL_thread.h>
#endif

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
#ifndef __EMSCRIPTEN__
        friend int networkThread(void *data);
#endif
        friend class MessageOut;

#ifndef __EMSCRIPTEN__
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
#endif

        void setError(const std::string &error);

        uint16_t readWord(int pos);

        /**
         * Drops the bytes requested by skip() from the front of the incoming
         * buffer. Called with mMutex held.
         */
        void applyToSkip();

        static const PacketInfo *findPacketInfo(uint16_t id);

#ifdef __EMSCRIPTEN__
        /**
         * Closes and releases the WebSocket, if there is one. Afterwards no
         * further events can arrive for this Network.
         */
        void closeSocket();

        static bool onOpen(int eventType,
                           const EmscriptenWebSocketOpenEvent *event,
                           void *userData);
        static bool onMessage(int eventType,
                              const EmscriptenWebSocketMessageEvent *event,
                              void *userData);
        static bool onError(int eventType,
                            const EmscriptenWebSocketErrorEvent *event,
                            void *userData);
        static bool onClose(int eventType,
                            const EmscriptenWebSocketCloseEvent *event,
                            void *userData);

        EMSCRIPTEN_WEBSOCKET_T mSocket = 0;
        ServerInfo mServer;
        bool mOpened = false;
#else
        void receive();

        TCPsocket mSocket = nullptr;
#endif

        char *mInBuffer, *mOutBuffer;
        unsigned int mInSize = 0;
#ifdef __EMSCRIPTEN__
        unsigned int mInCapacity = 0;
#endif
        unsigned int mOutSize = 0;

        unsigned int mToSkip = 0;

        int mState = IDLE;
        std::string mError;

#ifndef __EMSCRIPTEN__
        SDL_Thread *mWorkerThread = nullptr;
        std::shared_ptr<ConnectRequest> mConnectRequest;
#endif
        Mutex mMutex;

        std::map<uint16_t, MessageHandler *> mMessageHandlers;
};

} // namespace TmwAthena
