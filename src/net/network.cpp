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

#include "network.h"

#include "messagehandler.h"
#include "messagein.h"
#include "messageout.h"

#include "../log.h"

Network::Network():
    mClient(0), mServer(0),
    mAddress(), mPort(0),
    mState(IDLE)
{
}

Network::~Network()
{
    clearHandlers();

    if (mState != IDLE && mState != NET_ERROR)
        disconnect();
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
        logger->log("Empty address given to Network::connect()!");
        mState = NET_ERROR;
        return false;
    }

    logger->log("Network::Connecting to %s:%i", address.c_str(), port);

    mAddress = address;
    mPort = port;

    mState = CONNECTING;

    mClient = enet_host_create(NULL, 1, 0, 0);

    if (!mClient)
    {
        logger->error("An error occurred while trying to create an ENet client.");
    }

    ENetAddress enetAddress;

    enet_address_set_host(&enetAddress, address.c_str());
    enetAddress.port = port;

    // Initiate the connection, allocating channel 0.
    mServer = enet_host_connect(mClient, &enetAddress, 1);

    if (mServer == 0)
    {
        logger->log("Unable to initiate connection to the server.");
        mState = NET_ERROR;
        return false;
    }

    return true;
}

void Network::disconnect()
{
    mState = IDLE;

    if (mServer)
    {
        enet_peer_disconnect(mServer, 0);
        enet_host_flush(mClient);
        enet_peer_reset(mServer);
        mServer = 0;
    }
}

void Network::registerHandler(MessageHandler *handler)
{
    const Uint16 *i = handler->handledMessages;

    while(*i)
    {
        mMessageHandlers[*i] = handler;
        i++;
    }

    handler->setNetwork(this);
}

void Network::unregisterHandler(MessageHandler *handler)
{
    for (const Uint16 *i = handler->handledMessages; *i; i++)
    {
        mMessageHandlers.erase(*i);
    }

    handler->setNetwork(0);
}

void Network::clearHandlers()
{
    MessageHandlerIterator i;
    for (i = mMessageHandlers.begin(); i != mMessageHandlers.end(); i++)
    {
        i->second->setNetwork(0);
    }
    mMessageHandlers.clear();
}

void Network::dispatchMessages()
{
    while (!mIncomingPackets.empty())
    {
        ENetPacket *packet = mIncomingPackets.front();
        MessageIn msg((const char *)packet->data, packet->dataLength);

        MessageHandlerIterator iter = mMessageHandlers.find(msg.getId());

        printf("Received packet: %x\n", msg.getId());

        if (iter != mMessageHandlers.end())
            iter->second->handleMessage(&msg);
        else
            logger->log("Unhandled packet: %x", msg.getId());
        mIncomingPackets.pop();
        // Clean up the packet now that we're done using it.
        enet_packet_destroy(packet);
    }
}

void Network::flush()
{
    if (mState == IDLE || mState == NET_ERROR)
    {
        return;
    }

    ENetEvent event;

    // Wait up to 10 milliseconds for an event.
    while (enet_host_service(mClient, &event, 10) > 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                logger->log("Connected.");
                mState = CONNECTED;
                // Store any relevant server information here.
                event.peer->data = 0;
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                logger->log("Incoming data...");
                mIncomingPackets.push(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                mState = IDLE;
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

    // If connected, manage incoming and outcoming packets
    if (isConnected())
    {
        while (isConnected() && !mOutgoingPackets.empty())
        {
            ENetPacket *packet = mOutgoingPackets.front();
            enet_peer_send(mServer, 0, packet);
            mOutgoingPackets.pop();
        }

        dispatchMessages();
    }
}

void Network::send(MessageOut *msg)
{
    if (mState == IDLE || mState == NET_ERROR)
    {
        logger->log("Warning: attempt to send a message while network not "
                    "ready.");
        return;
    }

    ENetPacket *packet = enet_packet_create(msg->getData(),
                                            msg->getDataSize(),
                                            ENET_PACKET_FLAG_RELIABLE);
    mOutgoingPackets.push(packet);
}

char *iptostring(int address)
{
    static char asciiIP[16];

    sprintf(asciiIP, "%i.%i.%i.%i",
            (unsigned char)(address),
            (unsigned char)(address >> 8),
            (unsigned char)(address >> 16),
            (unsigned char)(address >> 24));

    return asciiIP;
}
