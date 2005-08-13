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

class Image;
class Sound;

#ifndef TMW_DATADIR
#define TMW_DATADIR ""
#endif

#ifdef ERROR
#undef ERROR
#endif

enum {
    EXIT,
    LOGIN,
    CHAR_SERVER,
    CHAR_SELECT,
    CHAR_NEW,
    CHAR_DEL,
    GAME,
    ERROR,
    UPDATE
};

/* length definitions for several char[]s in order
 * to be able to use strncpy instead of strcpy for
 * security and stability reasons
 */
#define LEN_USERNAME 25
#define LEN_PASSWORD 25

extern Image *login_wallpaper;
extern char username[25];
extern char password[25];
extern int map_address, char_ID;
extern short map_port;
extern char map_name[16];
extern int account_ID, session_ID1, session_ID2;
extern char sex, n_server, n_character;
extern unsigned char state;
extern Sound sound;
extern bool useOpenGL;

#endif
