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

#include <string>

#include "SDL_net.h"
#include "messagein.h"
#include "messageout.h"

/** Convert an address from int format to string */
char *iptostring(int address);

/** Open a session with a server */
int open_session(const char* address, short port);

/** Close a session */
void close_session();

/** Send and receive data waiting in the buffers */
void flush();

/**
 * Returns the next arriving message, waiting for it if necessary.
 */
MessageIn get_next_message();
extern char *out;


void writeByte(int pos, unsigned char value);//writeByte(char value);
void writeWord(int pos, unsigned short value);//writeWord(short value);
void writeLong(int pos, unsigned int value);//writeLong(int value);
char *writePointer(int pos); //writeString(const std::string &string, int length);
void writeSet(unsigned int value);
void skip(int len);

extern unsigned int in_size;   /**< Amount of data in input buffer. */
extern unsigned int out_size;  /**< Amount of data in output buffer. */

#endif
