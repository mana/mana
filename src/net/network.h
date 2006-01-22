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
#include <SDL_net.h>
#include <SDL_thread.h>

class MessageHandler;
class MessageIn;

class Network;

class Network
{
    public:
        friend int networkThread(void *data);
        friend class MessageOut;

        Network();
        ~Network();

        bool connect(const char *address, short port);
        void disconnect();

        void registerHandler(MessageHandler *handler);
        void unregisterHandler(MessageHandler *handler);
        void clearHandlers();

        int getState() const { return mState; }
        bool isConnected() const { return mState == CONNECTED; }

        int getInSize() const { return mInSize; }

        void skip(int len);

        bool messageReady();
        MessageIn getNextMessage();

        void dispatchMessages();
        void flush();

        enum {
            IDLE,
            CONNECTED,
            CONNECTING,
            DATA,
            ERROR
        };

    protected:
        Uint16 readWord(int pos);

        TCPsocket mSocket;

        char *mAddress;
        short mPort;

        char *mInBuffer, *mOutBuffer;
        unsigned int mInSize, mOutSize;

        unsigned int mToSkip;

        int mState;

        SDL_Thread *mWorkerThread;
        SDL_mutex *mMutex;

        std::map<Uint16, MessageHandler*> mMessageHandlers;

        bool realConnect();
        void receive();
};

/** Convert an address from int format to string */
char *iptostring(int address);

#endif
