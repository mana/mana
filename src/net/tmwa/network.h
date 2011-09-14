/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef NET_TA_NETWORK_H
#define NET_TA_NETWORK_H

#include "utils/mutex.h"

#include "net/serverinfo.h"

#include "net/tmwa/messagehandler.h"
#include "net/tmwa/messagein.h"
#include "net/tmwa/messageout.h"

#include <SDL_net.h>
#include <SDL_thread.h>

#include <map>
#include <string>

/**
 * Protocol version, reported to the eAthena char and mapserver who can adjust
 * the protocol accordingly.
 */
#define CLIENT_PROTOCOL_VERSION      1

namespace TmwAthena {

class Network
{
    public:
        Network();

        ~Network();

        bool connect(ServerInfo server);

        void disconnect();

        ServerInfo getServer() const
        { return mServer; }

        void registerHandler(MessageHandler *handler);

        void unregisterHandler(MessageHandler *handler);

        void clearHandlers();

        int getState() const { return mState; }

        const std::string &getError() const { return mError; }

        bool isConnected() const { return mState == CONNECTED; }

        int getInSize() const { return mInSize; }

        void skip(int len);

        bool messageReady();

        MessageIn getNextMessage();

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

    protected:
        friend int networkThread(void *data);
        friend class MessageOut;

        static Network *instance();

        void setError(const std::string &error);

        Uint16 readWord(int pos);

        bool realConnect();

        void receive();

        TCPsocket mSocket;

        ServerInfo mServer;

        char *mInBuffer, *mOutBuffer;
        unsigned int mInSize, mOutSize;

        unsigned int mToSkip;

        int mState;
        std::string mError;

        SDL_Thread *mWorkerThread;
        Mutex mMutex;

        typedef std::map<Uint16, MessageHandler*> MessageHandlers;
        typedef MessageHandlers::iterator MessageHandlerIterator;
        MessageHandlers mMessageHandlers;

        static Network *mInstance;
};

} // namespace TmwAthena

#endif // NET_TA_NETWORK_H
