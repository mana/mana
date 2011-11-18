/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAIN_H
#define MAIN_H

/**
 * \mainpage
 *
 * \section Introduction Introduction
 *
 * This is the documentation for the Mana client (http://manasource.org). It is
 * always a work in progress, with the intent to make it easier for new
 * developers to grow familiar with the source code.
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
 * The client supports two servers, \link EAthena eAthena\endlink (the TMW
 * version) and the \link ManaServ Mana server\endlink. To achieve this, the
 * \link Net network communication layer\endlink is abstracted in many
 * different interfaces, which have different implementations for each server.
 */

#ifdef HAVE_CONFIG_H
#include "../config.h"
#elif defined WIN32
#include "winver.h"
#elif defined __APPLE__
#define PACKAGE_VERSION "0.5.3"
#endif

#ifdef PACKAGE_VERSION
#define FULL_VERSION "v" PACKAGE_VERSION
#else
#define FULL_VERSION "Unknown Version"
#endif

#ifdef PACKAGE_OS
// If it's already been defined, let's not change it
#elif defined __APPLE__
#define PACKAGE_OS "Apple"
#elif defined __FreeBSD__ || defined __DragonFly__
#define PACKAGE_OS "FreeBSD"
#elif defined __NetBSD__
#define PACKAGE_OS "NetBSD"
#elif defined __OpenBSD__
#define PACKAGE_OS "OpenBSD"
#elif defined __linux__ || defined __linux
#define PACKAGE_OS "Linux"
#elif defined __GNU__
#define PACKAGE_OS "GNU Hurd"
#elif defined WIN32 || defined _WIN32 || defined __WIN32__ || defined __NT__ \
    || defined WIN64 || defined _WIN64 || defined __WIN64__
#define PACKAGE_OS "Windows"
#else
#define PACKAGE_OS "Other"
#endif

#ifdef PACKAGE_VERSION
#define PACKAGE_EXTENDED_VERSION "Mana/" PACKAGE_VERSION " (" PACKAGE_OS "; %s)"
#else
#define PACKAGE_EXTENDED_VERSION "Mana (" PACKAGE_OS "; %s)"
#endif

#ifndef PKG_DATADIR
#define PKG_DATADIR ""
#endif

#endif
