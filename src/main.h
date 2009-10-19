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

#ifndef MAIN_H
#define MAIN_H

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
#elif defined __APPLE__
#define PACKAGE_VERSION "0.0.29.1"
#endif

#ifdef PACKAGE_VERSION
#ifdef TMWSERV_SUPPORT
#define FULL_VERSION "v" PACKAGE_VERSION " (tmwserv)"
#else
#define FULL_VERSION "v" PACKAGE_VERSION " (eAthena)"
#endif
#else
#define FULL_VERSION "Unknown Version"
#endif

#ifndef PKG_DATADIR
#define PKG_DATADIR ""
#endif

#define MAX_CHARACTER_COUNT 3

#ifdef TWMSERV_SUPPORT
#define DEFAULT_PORT 9601
#else
#define DEFAULT_PORT 6901
#endif

/*
 * Client different States
 */
enum State {
    STATE_ERROR = -1,
    STATE_START = 0,
    STATE_CHOOSE_SERVER,
    STATE_CONNECT_SERVER,
    STATE_LOGIN,
    STATE_LOGIN_ATTEMPT,
    STATE_WORLD_SELECT, // 5
    STATE_WORLD_SELECT_ATTEMPT,
    STATE_UPDATE,
    STATE_LOAD_DATA,
    STATE_GET_CHARACTERS,
    STATE_CHAR_SELECT, // 10
    STATE_CONNECT_GAME,
    STATE_GAME,
    STATE_CHANGE_MAP, // Switch map-server/gameserver
    STATE_LOGIN_ERROR,
    STATE_ACCOUNTCHANGE_ERROR, // 15
    STATE_REGISTER,
    STATE_REGISTER_ATTEMPT,
    STATE_CHANGEPASSWORD,
    STATE_CHANGEPASSWORD_ATTEMPT,
    STATE_CHANGEPASSWORD_SUCCESS, // 20
    STATE_CHANGEEMAIL,
    STATE_CHANGEEMAIL_ATTEMPT,
    STATE_CHANGEEMAIL_SUCCESS,
    STATE_UNREGISTER,
    STATE_UNREGISTER_ATTEMPT, // 25
    STATE_UNREGISTER_SUCCESS,
    STATE_SWITCH_SERVER,
    STATE_SWITCH_LOGIN,
    STATE_SWITCH_CHARACTER,
    STATE_LOGOUT_ATTEMPT, // 30
    STATE_WAIT,
    STATE_EXIT,
    STATE_FORCE_QUIT
};

extern State state;
extern std::string errorMessage;

#endif
