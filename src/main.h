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

#ifndef _TMW_MAIN_H
#define _TMW_MAIN_H

#include <string>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#elif defined WIN32
#include "../The_Mana_World_private.h"
#define PACKAGE_VERSION PRODUCT_VERSION
#endif

#ifndef TMW_DATADIR
#define TMW_DATADIR ""
#endif


enum {
    EXIT_STATE,
    LOGIN_STATE,
    ACCOUNT_STATE,
    REGISTER_STATE,
    CHAR_CONNECT_STATE,
    CHAR_SERVER_STATE,
    CHAR_SELECT_STATE,
    CHAR_NEW_STATE,
    CHAR_DEL_STATE,
    GAME_STATE,
    ERROR_STATE,
    UPDATE_STATE,
    CONNECTING_STATE
};

/* length definitions for several char[]s in order
 * to be able to use strncpy instead of strcpy for
 * security and stability reasons
 */
enum {
    LEN_MAX_USERNAME     = 25,
    LEN_MIN_USERNAME     = 4,
    LEN_MAX_PASSWORD     = 25,
    LEN_MIN_PASSWORD     = 4
};

extern char n_server, n_character;
extern unsigned char state;
extern std::string errorMessage;

#endif
