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

#ifndef EA_NETWORK_H
#define EA_NETWORK_H

#include <SDL_net.h>
#include <SDL_thread.h>

#include <map>
#include <string>

/**
 * Protocol version, reported to the eAthena char and mapserver who can adjust
 * the protocol accordingly.
 */
#define CLIENT_PROTOCOL_VERSION      1

class MessageHandler;
class MessageIn;

class Network
{
    public:
        friend int networkThread(void *data);
        friend class MessageOut;

        Network();

        ~Network();

        bool connect(const std::string &address, short port);

        void disconnect();

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

    private:
        static Network *instance();

        void setError(const std::string &error);

        Uint16 readWord(int pos);

        bool realConnect();

        void receive();

        TCPsocket mSocket;

        std::string mAddress;
        short mPort;

        char *mInBuffer, *mOutBuffer;
        unsigned int mInSize, mOutSize;

        unsigned int mToSkip;

        int mState;
        std::string mError;

        SDL_Thread *mWorkerThread;
        SDL_mutex *mMutex;

        typedef std::map<Uint16, MessageHandler*> MessageHandlers;
        typedef MessageHandlers::iterator MessageHandlerIterator;
        MessageHandlers mMessageHandlers;

        static Network *mInstance;
};

#endif
