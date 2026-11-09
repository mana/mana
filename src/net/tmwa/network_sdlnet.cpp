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

#include "net/tmwa/network.h"

#include "log.h"

#include "utils/format.h"
#include "utils/gettext.h"
#include "utils/mutex.h"
#include "utils/stringutils.h"

#include <memory>
#include <string>

namespace TmwAthena {

int Network::networkThread(void *data)
{
    std::unique_ptr<std::shared_ptr<ConnectRequest>> ref(
                static_cast<std::shared_ptr<ConnectRequest>*>(data));
    ConnectRequest &request = **ref;
    const ServerInfo &server = request.server;

    // Resolve and connect without touching the Network, since it may be
    // destroyed in the meantime
    IPaddress ipAddress;
    TCPsocket socket = nullptr;
    std::string error;

    if (SDLNet_ResolveHost(&ipAddress, server.hostname.c_str(),
                           server.port) == -1)
    {
        error = strformat(_("Unable to resolve host \"{}\""),
                          server.hostname);
        Log::info("SDLNet_ResolveHost: {}", error);
    }
    else
    {
        socket = SDLNet_TCP_Open(&ipAddress);
        if (!socket)
        {
            error = SDLNet_GetError();
            Log::info("Error in SDLNet_TCP_Open(): {}", error);
        }
    }

    Network *network;
    {
        auto lockedNetwork = request.network.lock();
        network = *lockedNetwork;

        if (!network)
        {
            // The connection was abandoned
            if (socket)
                SDLNet_TCP_Close(socket);
            return 0;
        }

        if (!socket)
        {
            network->setError(error);
            return -1;
        }

        network->mSocket = socket;
        network->mState = CONNECTED;

        Log::info("Network::Started session with {}:{}",
                  ipToString(ipAddress.host), ipAddress.port);
    }

    network->receive();

    return 0;
}

void Network::initTransport()
{
    SDLNet_Init();
}

void Network::deinitTransport()
{
    SDLNet_Quit();
}

bool Network::connectTransport(const ServerInfo &server)
{
    mConnectRequest = std::make_shared<ConnectRequest>(server, this);

    // The thread holds its own reference, so that the request outlives the
    // Network when the thread is abandoned
    auto *ref = new std::shared_ptr<ConnectRequest>(mConnectRequest);

    mWorkerThread = SDL_CreateThread(networkThread, "Network", ref);
    if (!mWorkerThread)
    {
        delete ref;
        mConnectRequest.reset();
        setError("Unable to create network worker thread");
        return false;
    }

    return true;
}

void Network::disconnectTransport()
{
    if (mWorkerThread)
    {
        bool abandon;
        {
            // Opening a connection can take very long and can't be
            // interrupted, so rather than waiting for it, the thread is
            // abandoned and closes the socket itself once it is done.
            auto network = mConnectRequest->network.lock();
            abandon = mState == CONNECTING;
            if (abandon)
                *network = nullptr;

            // Makes the receive loop exit
            mState = IDLE;
        }

        if (abandon)
            SDL_DetachThread(mWorkerThread);
        else
            SDL_WaitThread(mWorkerThread, nullptr);

        mWorkerThread = nullptr;
        mConnectRequest.reset();
    }

    mState = IDLE;

    if (mSocket)
    {
        SDLNet_TCP_Close(mSocket);
        mSocket = nullptr;
    }
}

void Network::sendTransport(const char *data, size_t size)
{
    const int ret = SDLNet_TCP_Send(mSocket, data, size);
    if (ret < (int) size)
    {
        setError("Error in SDLNet_TCP_Send(): " +
                 std::string(SDLNet_GetError()));
    }
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

    char buffer[65536];

    while (mState == CONNECTED)
    {
        // TODO Try to get this to block all the time while still being able
        // to escape the loop
        int numReady = SDLNet_CheckSockets(set, ((Uint32)500));
        int ret;
        switch (numReady)
        {
            case -1:
                Log::error("SDLNet_CheckSockets");
                // FALLTHROUGH
            case 0:
                break;

            case 1:
            {
                // Receive data from the socket
                ret = SDLNet_TCP_Recv(mSocket, buffer, sizeof(buffer));

                if (!ret)
                {
                    // We got disconnected
                    mState = IDLE;
                    Log::info("Disconnected.");
                }
                else if (ret < 0)
                {
                    setError(_("Connection to server terminated. ") +
                             std::string(SDLNet_GetError()));
                }
                else
                {
                    received(buffer, ret);
                }
                break;
            }

            default:
                // more than one socket is ready..
                // this should not happen since we only listen once socket.
                setError(fmt::format("Error in SDLNet_TCP_Recv(), {} sockets "
                                     "are ready: {}",
                                     numReady, SDLNet_GetError()));
                break;
        }
    }

    if (SDLNet_TCP_DelSocket(set, mSocket) == -1)
    {
        Log::info("Error in SDLNet_DelSocket(): {}", SDLNet_GetError());
    }

    SDLNet_FreeSocketSet(set);
}

} // namespace TmwAthena
