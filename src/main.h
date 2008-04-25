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

/**
 * \mainpage
 *
 * \section Introduction Introduction
 *
 * This is the documentation for the client of The Mana World
 * (http://themanaworld.org). It is always a work in progress, with the intent
 * to make it easier for new developers to grow familiar with the source code.
 *
 * \section General General information
 *
 * During the game, the current Map is displayed by the main Viewport, which
 * is the bottom-most widget in the WindowContainer. Aside the viewport, the
 * window container keeps track of all the \link Window Windows\endlink
 * displayed during the game. It is the <i>top</i> widget for Guichan.
 *
 * A Map is composed of several layers of \link Image Images\endlink (tiles),
 * a layer with collision information and \link Sprite Sprites\endlink. The
 * sprites define the visible part of \link Being Beings\endlink and
 * \link FloorItem FloorItems\endlink, they are drawn from top to bottom
 * by the map, interleaved with the tiles in the fringe layer.
 *
 * The server is split up into an \link Net::AccountServer account
 * server\endlink, a \link Net::ChatServer chat server\endlink and a \link
 * Net::GameServer game server\endlink. There may be multiple game servers.
 * Handling of incoming messages is spread over several \link MessageHandler
 * MessageHanders\endlink.
 */

#include <string>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#elif defined WIN32
#include "winver.h"
#endif

#ifndef TMW_DATADIR
#define TMW_DATADIR ""
#endif

/*
 * Client different States
 */
enum {
    STATE_CHOOSE_SERVER,
    STATE_CONNECT_ACCOUNT,
    STATE_UPDATE,
    STATE_LOADDATA,
    STATE_LOGIN,
    STATE_LOGIN_ATTEMPT,
    STATE_LOGIN_ERROR,
    STATE_REGISTER,
    STATE_REGISTER_ATTEMPT,
    STATE_ACCOUNTCHANGE_ERROR,
    STATE_ERROR,
    STATE_CHAR_SELECT,
    STATE_CHANGEEMAIL_ATTEMPT,
    STATE_CHANGEEMAIL,
    STATE_CHANGEPASSWORD_ATTEMPT,
    STATE_CHANGEPASSWORD,
    STATE_UNREGISTER_ATTEMPT,
    STATE_UNREGISTER,
    STATE_SWITCH_CHARACTER,
    STATE_RECONNECT_ACCOUNT,
    STATE_SWITCH_ACCOUNTSERVER_ATTEMPT,
    STATE_SWITCH_ACCOUNTSERVER,
    STATE_LOGOUT_ATTEMPT,
    STATE_CONNECT_GAME,
    STATE_GAME,
    STATE_WAIT,
    STATE_EXIT,
    STATE_FORCE_QUIT
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

// Default game values
// -------------------
// Screen
const short defaultScreenWidth = 800;
const short defaultScreenHeight = 600;
// Sound
const short defaultSfxVolume = 100;
const short defaultMusicVolume = 60;
// Account Server Name and port
const std::string defaultAccountServerName = "testing.themanaworld.org";
const short defaultAccountServerPort = 9601;

// Defines the number of usable player slots
const short maxSlot = 2;

extern std::string token;
extern unsigned char state;
extern std::string errorMessage;

#endif
