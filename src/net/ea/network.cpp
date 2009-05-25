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

#include "net/ea/network.h"

#include "net/messagehandler.h"
#include "net/messagein.h"

#include "log.h"
#include "utils/stringutils.h"

#include <sstream>

/** Warning: buffers and other variables are shared,
    so there can be only one connection active at a time */

short packet_lengths[] = {
   10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
// #0x0040
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  50,  3, -1, 55, 17,  3, 37, 46, -1, 23, -1,  3,108,  3,  2,
    3, 28, 19, 11,  3, -1,  9,  5, 54, 53, 58, 60, 41,  2,  6,  6,
// #0x0080
    7,  3,  2,  2,  2,  5, 16, 12, 10,  7, 29, 23, -1, -1, -1,  0,
    7, 22, 28,  2,  6, 30, -1, -1,  3, -1, -1,  5,  9, 17, 17,  6,
   23,  6,  6, -1, -1, -1, -1,  8,  7,  6,  7,  4,  7,  0, -1,  6,
    8,  8,  3,  3, -1,  6,  6, -1,  7,  6,  2,  5,  6, 44,  5,  3,
// #0x00C0
    7,  2,  6,  8,  6,  7, -1, -1, -1, -1,  3,  3,  6,  6,  2, 27,
    3,  4,  4,  2, -1, -1,  3, -1,  6, 14,  3, -1, 28, 29, -1, -1,
   30, 30, 26,  2,  6, 26,  3,  3,  8, 19,  5,  2,  3,  2,  2,  2,
    3,  2,  6,  8, 21,  8,  8,  2,  2, 26,  3, -1,  6, 27, 30, 10,
// #0x0100
    2,  6,  6, 30, 79, 31, 10, 10, -1, -1,  4,  6,  6,  2, 11, -1,
   10, 39,  4, 10, 31, 35, 10, 18,  2, 13, 15, 20, 68,  2,  3, 16,
    6, 14, -1, -1, 21,  8,  8,  8,  8,  8,  2,  2,  3,  4,  2, -1,
    6, 86,  6, -1, -1,  7, -1,  6,  3, 16,  4,  4,  4,  6, 24, 26,
// #0x0140
   22, 14,  6, 10, 23, 19,  6, 39,  8,  9,  6, 27, -1,  2,  6,  6,
  110,  6, -1, -1, -1, -1, -1,  6, -1, 54, 66, 54, 90, 42,  6, 42,
   -1, -1, -1, -1, -1, 30, -1,  3, 14,  3, 30, 10, 43, 14,186,182,
   14, 30, 10,  3, -1,  6,106, -1,  4,  5,  4, -1,  6,  7, -1, -1,
// #0x0180
    6,  3,106, 10, 10, 34,  0,  6,  8,  4,  4,  4, 29, -1, 10,  6,
   90, 86, 24,  6, 30,102,  9,  4,  8,  4, 14, 10,  4,  6,  2,  6,
    3,  3, 35,  5, 11, 26, -1,  4,  4,  6, 10, 12,  6, -1,  4,  4,
   11,  7, -1, 67, 12, 18,114,  6,  3,  6, 26, 26, 26, 26,  2,  3,
// #0x01C0
    2, 14, 10, -1, 22, 22,  4,  2, 13, 97,  0,  9,  9, 29,  6, 28,
    8, 14, 10, 35,  6,  8,  4, 11, 54, 53, 60,  2, -1, 47, 33,  6,
   30,  8, 34, 14,  2,  6, 26,  2, 28, 81,  6, 10, 26,  2, -1, -1,
   -1, -1, 20, 10, 32,  9, 34, 14,  2,  6, 48, 56, -1,  4,  5, 10,
// #0x2000
   26,  0,  0,  0, 18,  0,  0,  0,  0,  0,  0, 19,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

const unsigned int BUFFER_SIZE = 65536;

int networkThread(void *data)
{
    Network *network = static_cast<Network*>(data);

    if (!network->realConnect())
        return -1;

    network->receive();

    return 0;
}

Network *Network::mInstance = 0;

Network::Network():
    mSocket(0),
    mAddress(), mPort(0),
    mInBuffer(new char[BUFFER_SIZE]),
    mOutBuffer(new char[BUFFER_SIZE]),
    mInSize(0), mOutSize(0),
    mToSkip(0),
    mState(IDLE),
    mWorkerThread(0)
{
    SDLNet_Init();

    mMutex = SDL_CreateMutex();
    mInstance = this;
}

Network::~Network()
{
    clearHandlers();

    if (mState != IDLE && mState != NET_ERROR)
        disconnect();

    SDL_DestroyMutex(mMutex);
    mInstance = 0;

    delete[] mInBuffer;
    delete[] mOutBuffer;

    SDLNet_Quit();
}

bool Network::connect(const std::string &address, short port)
{
    if (mState != IDLE && mState != NET_ERROR)
    {
        logger->log("Tried to connect an already connected socket!");
        return false;
    }

    if (address.empty())
    {
        setError("Empty address given to Network::connect()!");
        return false;
    }

    logger->log("Network::Connecting to %s:%i", address.c_str(), port);

    mAddress = address;
    mPort = port;

    // Reset to sane values
    mOutSize = 0;
    mInSize = 0;
    mToSkip = 0;

    mState = CONNECTING;
    mWorkerThread = SDL_CreateThread(networkThread, this);
    if (!mWorkerThread)
    {
        setError("Unable to create network worker thread");
        return false;
    }

    return true;
}

void Network::disconnect()
{
    mState = IDLE;

    if (mWorkerThread)
    {
        SDL_WaitThread(mWorkerThread, NULL);
        mWorkerThread = NULL;
    }

    if (mSocket)
    {
        SDLNet_TCP_Close(mSocket);
        mSocket = 0;
    }
}

void Network::registerHandler(MessageHandler *handler)
{
    for (const Uint16 *i = handler->handledMessages; *i; ++i)
    {
        mMessageHandlers[*i] = handler;
    }

    handler->setNetwork(this);
}

void Network::unregisterHandler(MessageHandler *handler)
{
    for (const Uint16 *i = handler->handledMessages; *i; ++i)
    {
        mMessageHandlers.erase(*i);
    }

    handler->setNetwork(0);
}

void Network::clearHandlers()
{
    MessageHandlerIterator i;
    for (i = mMessageHandlers.begin(); i != mMessageHandlers.end(); ++i)
    {
        i->second->setNetwork(0);
    }
    mMessageHandlers.clear();
}

void Network::dispatchMessages()
{
    while (messageReady())
    {
        MessageIn msg = getNextMessage();

        MessageHandlerIterator iter = mMessageHandlers.find(msg.getId());

        if (iter != mMessageHandlers.end())
            iter->second->handleMessage(msg);
        else
            logger->log("Unhandled packet: %x", msg.getId());

        skip(msg.getLength());
    }
}

void Network::flush()
{
    if (!mOutSize || mState != CONNECTED)
        return;

    int ret;


    SDL_mutexP(mMutex);
    ret = SDLNet_TCP_Send(mSocket, mOutBuffer, mOutSize);
    if (ret < (int)mOutSize)
    {
        setError("Error in SDLNet_TCP_Send(): " +
                 std::string(SDLNet_GetError()));
    }
    mOutSize = 0;
    SDL_mutexV(mMutex);
}

void Network::skip(int len)
{
    SDL_mutexP(mMutex);
    mToSkip += len;
    if (!mInSize)
    {
        SDL_mutexV(mMutex);
        return;
    }

    if (mInSize >= mToSkip)
    {
        mInSize -= mToSkip;
        memmove(mInBuffer, mInBuffer + mToSkip, mInSize);
        mToSkip = 0;
    }
    else
    {
        mToSkip -= mInSize;
        mInSize = 0;
    }
    SDL_mutexV(mMutex);
}

bool Network::messageReady()
{
    int len = -1;

    SDL_mutexP(mMutex);
    if (mInSize >= 2)
    {
        len = packet_lengths[readWord(0)];

        if (len == -1 && mInSize > 4)
            len = readWord(2);

    }

    bool ret = (mInSize >= static_cast<unsigned int>(len));
    SDL_mutexV(mMutex);

    return ret;
}

MessageIn Network::getNextMessage()
{
    while (!messageReady())
    {
        if (mState == NET_ERROR)
            break;
    }

    SDL_mutexP(mMutex);
    int msgId = readWord(0);
    int len = packet_lengths[msgId];

    if (len == -1)
        len = readWord(2);

#ifdef DEBUG
    logger->log("Received packet 0x%x of length %d", msgId, len);
#endif

    MessageIn msg(mInBuffer, len);
    SDL_mutexV(mMutex);

    return msg;
}

bool Network::realConnect()
{
    IPaddress ipAddress;

    if (SDLNet_ResolveHost(&ipAddress, mAddress.c_str(), mPort) == -1)
    {
        std::string error = "Unable to resolve host \"" + mAddress + "\"";
        setError(error);
        logger->log("SDLNet_ResolveHost: %s", error.c_str());
        return false;
    }

    mState = CONNECTING;

    mSocket = SDLNet_TCP_Open(&ipAddress);
    if (!mSocket)
    {
        logger->log("Error in SDLNet_TCP_Open(): %s", SDLNet_GetError());
        setError(SDLNet_GetError());
        return false;
    }

    logger->log("Network::Started session with %s:%i",
                ipToString(ipAddress.host), ipAddress.port);

    mState = CONNECTED;

    return true;
}

void Network::receive()
{
    SDLNet_SocketSet set;

    if (!(set = SDLNet_AllocSocketSet(1)))
    {
        setError("Error in SDLNet_AllocSocketSet(): " +
                 std::string(SDLNet_GetError()));
        return;
    }

    if (SDLNet_TCP_AddSocket(set, mSocket) == -1)
    {
        setError("Error in SDLNet_AddSocket(): " +
                 std::string(SDLNet_GetError()));
    }

    while (mState == CONNECTED)
    {
        // TODO Try to get this to block all the time while still being able
        // to escape the loop
        int numReady = SDLNet_CheckSockets(set, ((Uint32)500));
        int ret;
        switch (numReady)
        {
            case -1:
                logger->log("Error: SDLNet_CheckSockets");
                // FALLTHROUGH
            case 0:
                break;

            case 1:
                // Receive data from the socket
                SDL_mutexP(mMutex);
                ret = SDLNet_TCP_Recv(mSocket, mInBuffer + mInSize, BUFFER_SIZE - mInSize);

                if (!ret)
                {
                    // We got disconnected
                    mState = IDLE;
                    logger->log("Disconnected.");
                }
                else if (ret < 0)
                {
                    setError("Error in SDLNet_TCP_Recv(): " +
                             std::string(SDLNet_GetError()));
                }
                else {
                    mInSize += ret;
                    if (mToSkip)
                    {
                        if (mInSize >= mToSkip)
                        {
                            mInSize -= mToSkip;
                            memmove(mInBuffer, mInBuffer + mToSkip, mInSize);
                            mToSkip = 0;
                        }
                        else
                        {
                            mToSkip -= mInSize;
                            mInSize = 0;
                        }
                    }
                }
                SDL_mutexV(mMutex);
                break;

            default:
                // more than one socket is ready..
                // this should not happen since we only listen once socket.
                std::stringstream errorStream;
                errorStream << "Error in SDLNet_TCP_Recv(), " << numReady
                            << " sockets are ready: " << SDLNet_GetError();
                setError(errorStream.str());
                break;
        }
    }

    if (SDLNet_TCP_DelSocket(set, mSocket) == -1)
    {
        logger->log("Error in SDLNet_DelSocket(): %s", SDLNet_GetError());
    }

    SDLNet_FreeSocketSet(set);
}

Network *Network::instance()
{
    return mInstance;
}

void Network::setError(const std::string &error)
{
    logger->log("Network error: %s", error.c_str());
    mError = error;
    mState = NET_ERROR;
}

Uint16 Network::readWord(int pos)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    return SDL_Swap16((*(Uint16*)(mInBuffer+(pos))));
#else
    return (*(Uint16*)(mInBuffer+(pos)));
#endif
}
