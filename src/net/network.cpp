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
#ifndef WIN32
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif

/** Warning: buffers and other variables are shared,
    so there can be only one connection active at a time */

int buffer_size = 65536;
char *in, *out;
int in_size, out_size;

SOCKET sock;
SOCKADDR_IN addr;
// File descriptors attached to socket
fd_set read_socket;
fd_set write_socket;

void WFIFOSET(int len)
{
    if (out_size + len >= buffer_size) {
        log("Warning: Output buffer full");
    }
    else {
        out_size += len;
    }
}

char *iptostring(int address)
{
    short temp1, temp2;
    static char asciiIP[16];

    temp1 = LOWORD(address);
    temp2 = HIWORD(address);
    sprintf(asciiIP, "%i.%i.%i.%i", LOBYTE(temp1), HIBYTE(temp1), LOBYTE(temp2), HIBYTE(temp2));
    return asciiIP;
}

SOCKET open_session(const char* address, short port)
{
    #ifdef WIN32
    WSADATA wsda;
    #endif
    struct hostent *server;
    int ret;

    // Init WinSock and connect the socket
    #ifdef WIN32
    WSAStartup(MAKEWORD(2,0), &wsda);
    #endif

    sock = socket(PF_INET, SOCK_STREAM, 0); // Create socket for current session
    if(sock==SOCKET_ERROR)return SOCKET_ERROR;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address);
    if(addr.sin_addr.s_addr == INADDR_NONE){
        server = NULL;
        server = gethostbyname(address);
        if(server == NULL)return SOCKET_ERROR;
            memcpy(&addr.sin_addr, server->h_addr_list[0], server->h_length);
    }

    ret = connect(sock, (struct sockaddr *) &addr, sizeof(addr));
    if(ret == SOCKET_ERROR)return SOCKET_ERROR;

    // Init buffers
    in = (char *)malloc(buffer_size);
    out = (char *)malloc(buffer_size);
    memset(in, '\0', buffer_size);
    memset(out, '\0', buffer_size);
    in_size = 0;
    out_size = 0;
    FD_CLR(sock, &read_socket);
    FD_CLR(sock, &write_socket);

    return sock;
}

void close_session()
{
    FD_CLR(sock,&read_socket);
    FD_CLR(sock,&write_socket);
    closesocket(sock);
    if(in!=NULL) {
        free(in);
    }
    if(out!=NULL) {
        free(out);
    }
    in = NULL;
    out = NULL;
    in_size = 0;
    out_size = 0;
    WSACleanup();
}

void flush()
{
    int ret = 0;
    void *buf = out;
    timeval time_out;

    // Init the time_out struct to 0s so it won't block
    time_out.tv_sec=0;
    time_out.tv_usec=0;

    // Clear file descriptors and set them to socket
    FD_ZERO(&read_socket);
    FD_ZERO(&write_socket);
    FD_SET(sock, &read_socket);
    FD_SET(sock, &write_socket);

    // Check if socket has available data by evaluating attached file descriptors
    select(FD_SETSIZE, &read_socket, &write_socket, NULL, &time_out);

    // Send data if available
    if(FD_ISSET(sock, &write_socket)) {
        // While there wasn't a error or sent the whole data: handles partial packet send
        while((ret!=SOCKET_ERROR)&&(out_size>0)) {
            ret = send(sock, (char *)buf, out_size, 0);

            if(ret!=SOCKET_ERROR && ret>0) {
                buf = (char*)buf+ret;
                out_size -= ret;
            }
        }
        if (ret == SOCKET_ERROR) {
            error("Socket Error");
#ifdef WIN32
            log("Error: Socket error: %i ", WSAGetLastError());
            if (WSAGetLastError() == 10053)
            log("Error: Packet size error");
            /** Probably the last packet you sent, was defined with
             *  wrong size: WFIFOSET(size);
             */
#else
            log("Error: Undefined socket error");
#endif
        }
    }

    // Read data, if available
    if(FD_ISSET(sock, &read_socket)) {
        /* There's no check for partial received packets because at this level
           the app doesn't know packet length, but it will done when parsing received data */
        ret = recv(sock, in+in_size, RFIFOSPACE, 0);
        if(ret==SOCKET_ERROR) {
#ifdef WIN32
            log("Error: Socket error: %i ", WSAGetLastError());
#else
            log("Error: Undefined socket error");
#endif
        } else RFIFOSET(ret); // Set size of available data to read
    }
}