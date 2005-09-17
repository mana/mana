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

#include <cassert>
#include <sstream>
#include <SDL_net.h>

#include "messagein.h"

#include "../log.h"
#ifdef MACOSX
#include "win2mac.h"
#endif

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
    0,  0,  0,  0, 55, 17,  3, 37, 46, -1, 23, -1,  3,108,  3,  2,
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
// #0x200
   26,  0,  0,  0, 18,  0,  0,  0,  0,  0,  0, 19,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

unsigned int buffer_size = 65536;
char *in = NULL;
char *out = NULL;
unsigned int in_size = 0;
unsigned int out_size = 0;
bool connectionOpen = false;

TCPsocket sock;
SDLNet_SocketSet set;

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

int open_session(const char* address, short port)
{
    assert(!connectionOpen);

    // Initialize SDL_net
    if (SDLNet_Init() == -1)
    {
        logger->log("Error in SDLNet_Init(): %s", SDLNet_GetError());
        return -1;
    }

    IPaddress ip;

    // Resolve host name
    if (SDLNet_ResolveHost(&ip, address, port) == -1)
    {
        logger->log("Error in SDLNet_ResolveHost(): %s", SDLNet_GetError());
        return -1;
    }

    // Create the socket for the current session
    sock = SDLNet_TCP_Open(&ip);
    if (!sock)
    {
        logger->log("Error in SDLNet_TCP_Open(): %s", SDLNet_GetError());
        return -1;
    }

    // Create a socket set to listen to socket
    set = SDLNet_AllocSocketSet(1);
    if (!set)
    {
        logger->log("Error in SDLNet_AllocSocketSet(): %s", SDLNet_GetError());
        return -1;
    }

    // Add the socket to the set
    int ret = SDLNet_TCP_AddSocket(set, sock);
    if (ret == -1)
    {
        logger->log("Error in SDLNet_AddSocket(): %s", SDLNet_GetError());
        return -1;
    }

    // Init buffers
    in = (char*)malloc(buffer_size);
    out = (char*)malloc(buffer_size);
    memset(in, '\0', buffer_size);
    memset(out, '\0', buffer_size);
    in_size = 0;
    out_size = 0;

    logger->log("Network::Started session with %s:%i", address, port);
    connectionOpen = true;

    return 0;
}

void close_session()
{
    assert(connectionOpen);

    // Remove the socket from the socket set
    int ret = SDLNet_TCP_DelSocket(set, sock);
    if (ret == -1)
    {
        logger->log("Error in SDLNet_DelSocket(): %s", SDLNet_GetError());
    }

    // Close the TCP connection
    SDLNet_TCP_Close(sock);

    // Free the socket set
    SDLNet_FreeSocketSet(set);
    set = NULL;

    // Clear buffers
    if (in != NULL)
    {
        free(in);
        in = NULL;
    }

    if (out != NULL)
    {
        free(out);
        out = NULL;
    }

    in_size = 0;
    out_size = 0;

    // Shutdown the network API
    SDLNet_Quit();

    logger->log("Network::Closed session");
    connectionOpen = false;
}

void flush()
{
    int numReady = SDLNet_CheckSockets(set, 0);
    if (numReady == -1)
    {
        logger->log("Error: SDLNet_CheckSockets");
        return;
    }
    else if (numReady)
    {
        // Receive data from the socket
        int ret = SDLNet_TCP_Recv(sock, in + in_size, buffer_size - in_size);
        if (ret <= 0)
        {
            logger->log("Warning: unknown error when receiving data");
            logger->log("SDLNet_GetError(): %s", SDLNet_GetError());
            // The client disconnected, notify it somewhere
            logger->error("Disconnected from server");
            return;
        }
        else {
            in_size += ret;
        }
    }

    // Send all available data, waits if not all data can be sent immediately
    if (out_size > 0)
    {
        int ret = SDLNet_TCP_Send(sock, (char*)out, out_size);
        if (ret < (int)out_size)
        {
            // It is likely that the server disconnected
            std::stringstream ss;
            ss << "Error in SDLNet_TCP_Send(): " << SDLNet_GetError();
            logger->error(ss.str());
            return;
        }
        out_size -= ret;
    }
}

unsigned short readWord(int pos)
{
#ifdef MACOSX
    return DR_SwapTwoBytes((*(unsigned short*)(in+(pos))));
#else
    return (*(unsigned short *)(in+(pos)));
#endif
}

MessageIn
get_next_message()
{
    // At least 2 bytes should be received for the message ID
    while (in_size < 2) flush();

    int length = packet_lengths[readWord(0)];

    if (length == -1)
    {
        // Another 2 bytes should be received for the length
        while (in_size < 4) flush();
        length = readWord(2);
    }

#ifdef DEBUG
    printf("Received packet 0x%x of length %d\n", readWord(0), length);
#endif

    // Make sure the whole packet is received
    while (in_size < (unsigned int)length) flush();

    return MessageIn(in, length);
}

void writeSet(unsigned int value)
{
    if (out_size + value >= buffer_size) {
        logger->log("Warning: Output buffer full");
    }
    else {
        out_size += value;
    }
}

void skip(int len)
{
    memcpy(in, in + len, in_size - len);
    in_size -= len;
}
