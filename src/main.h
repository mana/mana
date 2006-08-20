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

#if (defined __USE_UNIX98 || defined __FreeBSD__)
#include "../config.h"
#elif defined WIN32
#include "../The_Mana_World_private.h"
#define PACKAGE_VERSION PRODUCT_VERSION
#endif

#ifndef TMW_DATADIR
#define TMW_DATADIR ""
#endif


enum {
    STATE_CHOOSE_SERVER,
    STATE_CONNECT_ACCOUNT,
    STATE_UPDATE,
    STATE_LOGIN,
    STATE_LOGIN_ATTEMPT,
    STATE_REGISTER,
    STATE_REGISTER_ATTEMPT,
    STATE_CHAR_SELECT,
    STATE_ERROR,
    STATE_CONNECT_GAME,
    STATE_GAME,
    STATE_EXIT
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

extern char n_character;
extern std::string token;
extern unsigned char state;
extern std::string errorMessage;

#endif
