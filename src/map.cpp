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

#include "main.h"
#include "map.h"
#include "log.h"
#include "being.h"

#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#else
#include "./net/win2linux.h"
#endif

MAP tiled_map;

bool load_map(char *map_file) {
    PACKFILE *file = pack_fopen(map_file, "rp");
    if (!file) {
        warning(map_file);
        return false;
    }
    pack_fread(&tiled_map, sizeof(MAP), file);
    pack_fclose(file);
    return true;
}

void set_walk(short x_c, short y_c, bool walkable) {
    if (walkable == true) tiled_map.tiles[x_c][y_c].data[3] |= 0x0002;
    else tiled_map.tiles[x_c][y_c].data[3] &= 0x00fd;
}

bool get_walk(short x_c, short y_c) {
	bool ret = (tiled_map.tiles[x_c][y_c].data[3] & 0x0002)>0;
	if(ret==true) {
    NODE *node = get_head();
		while(node && ret==true) {
			if(get_x(node->coordinates)==x_c && get_y(node->coordinates)==y_c)
				ret = false;
			node = node->next;
		}
		return ret;
	} else return false;
}

unsigned char get_path_walk(unsigned short x, unsigned short y) {
    if (get_walk(x, y)) return 0;
    else return 1;
}

bool get_anim(short x_c, short y_c, char layer) {
    char temp = tiled_map.tiles[x_c][y_c].flags & 0x00C0;
    temp >>= 6;
    if(abs(temp)==layer)return (tiled_map.tiles[x_c][y_c].data[3] & 0x0001)>0;
    else return false;
}

void set_tile(short x_c, short y_c, char layer, unsigned short id) {
    if (layer == 0) {
        id <<= 6;
        tiled_map.tiles[x_c][y_c].data[0] = HIBYTE(id);
        tiled_map.tiles[x_c][y_c].data[1] &= 0x003f;
        tiled_map.tiles[x_c][y_c].data[1] |= LOBYTE(id);
    } else if (layer == 1) {
        id <<= 4;
        tiled_map.tiles[x_c][y_c].data[1] &= 0x00c0;
        tiled_map.tiles[x_c][y_c].data[1] |= HIBYTE(id);
        tiled_map.tiles[x_c][y_c].data[2] &= 0x000f;
        tiled_map.tiles[x_c][y_c].data[2] |= LOBYTE(id);
    } else if (layer == 2) {
        id <<= 2;
        tiled_map.tiles[x_c][y_c].data[2] &= 0x00f0;
        tiled_map.tiles[x_c][y_c].data[2] |= HIBYTE(id);
        tiled_map.tiles[x_c][y_c].data[3] &= 0x0003;
        tiled_map.tiles[x_c][y_c].data[3] |= LOBYTE(id);
    }
}

unsigned short get_tile(short x_c, short y_c, char layer) {
    unsigned short id = 0;
    if (layer == 0) {
        id = MAKEWORD(tiled_map.tiles[x_c][y_c].data[1] & 0x00c0,
                tiled_map.tiles[x_c][y_c].data[0]);
        id >>= 6;
    } else if (layer == 1) {
        id = MAKEWORD(tiled_map.tiles[x_c][y_c].data[2] & 0x00f0,
                tiled_map.tiles[x_c][y_c].data[1] & 0x003f);
        id >>= 4;
    } else if (layer == 2) {
        id = MAKEWORD(tiled_map.tiles[x_c][y_c].data[3] & 0x00fc,
                tiled_map.tiles[x_c][y_c].data[2] & 0x000f);
        id >>= 2;
    }
    return id;
}
