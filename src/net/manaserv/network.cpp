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

#include "net/manaserv/network.h"

#include "log.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/internal.h"
#include "net/manaserv/messagehandler.h"
#include "net/manaserv/messagein.h"

#include <enet/enet.h>

#include <map>

/**
 * The local host which is shared for all outgoing connections.
 */
namespace {
    ENetHost *client;
}

namespace ManaServ
{

typedef std::map<unsigned short, MessageHandler*> MessageHandlers;
typedef MessageHandlers::iterator MessageHandlerIterator;
static MessageHandlers mMessageHandlers;

void initialize()
{
    if (enet_initialize())
    {
        logger->error("Failed to initialize ENet.");
    }

#if defined(ENET_VERSION) && ENET_VERSION >= ENET_CUTOFF
    client = enet_host_create(NULL, 3, 0, 0, 0);
#else
    client = enet_host_create(NULL, 3, 0, 0);
#endif

    if (!client)
    {
        logger->error("Failed to create the local host.");
    }
}

void finalize()
{
    if (!client)
        return; // Wasn't initialized at all

    if (connections)
    {
        logger->error("Tried to shutdown the network subsystem while there "
                "are network connections left!");
    }

    clearNetworkHandlers();
    enet_deinitialize();
}

Connection *getConnection()
{
    if (!client)
    {
        logger->error("Tried to instantiate a network object before "
                "initializing the network subsystem!");
    }

    return new Connection(client);
}

void registerHandler(MessageHandler *handler)
{
    for (const Uint16 *i = handler->handledMessages; *i; i++)
    {
        mMessageHandlers[*i] = handler;
    }
}

void unregisterHandler(MessageHandler *handler)
{
    for (const Uint16 *i = handler->handledMessages; *i; i++)
    {
        mMessageHandlers.erase(*i);
    }
}

void clearNetworkHandlers()
{
    mMessageHandlers.clear();
}


/**
 * Dispatches a message to the appropriate message handler and
 * destroys it afterwards.
 */
namespace
{
    void dispatchMessage(ENetPacket *packet)
    {
        MessageIn msg((const char *)packet->data, packet->dataLength);

        MessageHandlerIterator iter = mMessageHandlers.find(msg.getId());

        if (iter != mMessageHandlers.end())
        {
            //logger->log("Received packet %x (%i B)",
            //        msg.getId(), msg.getLength());
            iter->second->handleMessage(msg);
        }
        else
        {
            logger->log("Unhandled packet %x (%i B)",
                    msg.getId(), msg.getLength());
        }

        // Clean up the packet now that we're done using it.
        enet_packet_destroy(packet);
    }
}

void flush()
{
    ENetEvent event;

    // Check if there are any new events
    while (enet_host_service(client, &event, 0) > 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                logger->log("Connected to port %d.", event.peer->address.port);
                // Store any relevant server information here.
                event.peer->data = 0;
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                dispatchMessage(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                logger->log("Disconnected.");
                // Reset the server information.
                event.peer->data = 0;
                break;

            case ENET_EVENT_TYPE_NONE:
            default:
                break;
        }
    }
}

}
