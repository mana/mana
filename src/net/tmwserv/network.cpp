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

#include "net/tmwserv/network.h"

#include "net/tmwserv/connection.h"
#include "net/tmwserv/internal.h"

#include "net/messagehandler.h"
#include "net/messagein.h"

#include "log.h"

#include <enet/enet.h>

#include <map>

/**
 * The local host which is shared for all outgoing connections.
 */
namespace {
    ENetHost *client;
}

typedef std::map<unsigned short, MessageHandler*> MessageHandlers;
typedef MessageHandlers::iterator MessageHandlerIterator;
static MessageHandlers mMessageHandlers;

void Net::initialize()
{
    if (enet_initialize())
    {
        logger->error("Failed to initialize ENet.");
    }

    client = enet_host_create(NULL, 3, 0, 0);

    if (!client)
    {
        logger->error("Failed to create the local host.");
    }
}

void Net::finalize()
{
    if (!client)
        return; // Wasn't initialized at all

    if (Net::connections) {
        logger->error("Tried to shutdown the network subsystem while there "
                "are network connections left!");
    }

    clearHandlers();
    enet_deinitialize();
}

Net::Connection *Net::getConnection()
{
    if (!client)
    {
        logger->error("Tried to instantiate a network object before "
                "initializing the network subsystem!");
    }

    return new Net::Connection(client);
}

void Net::registerHandler(MessageHandler *handler)
{
    for (const Uint16 *i = handler->handledMessages; *i; i++)
    {
        mMessageHandlers[*i] = handler;
    }
}

void Net::unregisterHandler(MessageHandler *handler)
{
    for (const Uint16 *i = handler->handledMessages; *i; i++)
    {
        mMessageHandlers.erase(*i);
    }
}

void Net::clearHandlers()
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

        if (iter != mMessageHandlers.end()) {
            //logger->log("Received packet %x (%i B)",
            //        msg.getId(), msg.getLength());
            iter->second->handleMessage(msg);
        }
        else {
            logger->log("Unhandled packet %x (%i B)",
                    msg.getId(), msg.getLength());
        }

        // Clean up the packet now that we're done using it.
        enet_packet_destroy(packet);
    }
}

void Net::flush()
{
    ENetEvent event;

    // Wait up to 10 milliseconds for an event.
    while (enet_host_service(client, &event, 10) > 0)
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
                logger->log("No event during 10 milliseconds.");
                break;

            default:
                logger->log("Unhandled enet event.");
                break;
        }
    }
}
