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

#ifndef _TMW_NETWORK_H
#define _TMW_NETWORK_H

#ifndef WIN32
#include "win2linux.h"
#else
#include <winsock.h>
#endif

#include <stdio.h>
#include "../log.h"

#ifdef MACOSX
#include "win2mac.h"
#endif

/** Macros to write in output buffer, pos is the location where to write data
    After you wrote len bytes, you have to use WFIFOSET */
#define WFIFOSPACE (buffer_size-out_size) // Number of bytes currently written in uotput buffer
#define WFIFOP(pos) (out+(pos+out_size)) // Return a pointer to a specific location in output buffer
#define WFIFOB(pos) (*(unsigned char *)(out+pos+out_size)) // Write a byte (1 byte)
#define WFIFOW(pos) (*(unsigned short *)(out+pos+out_size)) // Write a word (2 byte)
#define WFIFOL(pos) (*(unsigned int *)(out+pos+out_size)) // Write a long (4 byte)
//#define WFIFOSET(len) out_size+=len // Increase size of written data

#ifdef MACOSX
	#define net_b_value(id) (id)
	#define net_w_value(id) DR_SwapTwoBytes(id)
	#define net_l_value(id) DR_SwapFourBytes(id)
#else
	#define net_b_value(id) (id)
	#define net_w_value(id) (id)
	#define net_l_value(id) (id)
#endif


/** Macros to read from input buffer, pos is the location of data to be read
    After you read len bytes, you should use RFIFOSKIP */
#define RFIFOP(pos) (in+(pos)) // Get a pointer from a specific location in input buffer

#ifdef MACOSX
	#define RFIFOB(pos) ((*(unsigned char*)(in+(pos)))) // Read a byte
	#define RFIFOW(pos) DR_SwapTwoBytes((*(unsigned short*)(in+(pos)))) // Read a word
	#define RFIFOL(pos) DR_SwapFourBytes((*(unsigned int*)(in+(pos)))) // Read a long
#else
	#define RFIFOB(pos) (*(unsigned char*)(in+(pos))) // Read a byte
	#define RFIFOW(pos) (*(unsigned short*)(in+(pos))) // Read a word
	#define RFIFOL(pos) (*(unsigned int*)(in+(pos))) // Read a long
#endif

#define RFIFOSKIP(len) (memcpy(in,in+len,in_size-len));in_size-=len; // Empty len bytes from input buffer
#define RFIFOSPACE (buffer_size-in_size) // Return input buffer size
#define RFIFOSET(len) in_size+=len;

void WFIFOSET(int len);
char *iptostring(int address);
SOCKET open_session(const char* address, short port);
void close_session();
void flush();

extern char *in, *out; // Input, output buffer
extern int in_size, out_size; // Input, output buffer size

#endif
