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
 */

#include "map.h"
#include "log.h"
#include "being.h"

#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#else
#include "./net/win2linux.h"
#endif

MAP map;

/** Loads a map file */
bool load_map(char *map_file) {
	PACKFILE *file = pack_fopen(map_file, "rp");
	if(!file) {
		warning(map_file);
		return false;
	}
	pack_fread(&map, sizeof(MAP), file);
	pack_fclose(file);
	return true;
}


/** Set walkability flag for a tile */
void set_walk(short x_c, short y_c, bool walkable) {
	if(walkable==true)map.tiles[x_c][y_c].data[3] |= 0x0002;
	else map.tiles[x_c][y_c].data[3] &= 0x00fd;
}

/** Tell if a tile is walkable or not */
bool get_walk(short x_c, short y_c) {
	bool ret = (map.tiles[x_c][y_c].data[3] & 0x0002)>0;
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

/** Tell if a tile is walkable or not (0=walkable,1=not walkable) */
unsigned char get_path_walk(unsigned short x, unsigned short y) {
	if(get_walk(x, y))return 0;
	else return 1;
}

/** Tell if a tile is animated or not */
bool get_anim(short x_c, short y_c, char layer) {
	char temp = map.tiles[x_c][y_c].flags & 0x00C0;
	temp>>=6;
	if(abs(temp)==layer)return (map.tiles[x_c][y_c].data[3] & 0x0001)>0;
	else return false;
}

/** Set tile ID */
void set_tile(short x_c, short y_c, char layer, unsigned short id) {
	if(layer==0) {
		id <<= 6;
        map.tiles[x_c][y_c].data[0] = HIBYTE(id);
		map.tiles[x_c][y_c].data[1] &= 0x003f;
        map.tiles[x_c][y_c].data[1] |= LOBYTE(id);
	} else if(layer==1) {
		id  <<= 4;
		map.tiles[x_c][y_c].data[1] &= 0x00c0;
		map.tiles[x_c][y_c].data[1] |= HIBYTE(id);
		map.tiles[x_c][y_c].data[2] &= 0x000f;
		map.tiles[x_c][y_c].data[2] |= LOBYTE(id);
	} else if(layer==2) {
		id <<= 2;
		map.tiles[x_c][y_c].data[2] &= 0x00f0;
		map.tiles[x_c][y_c].data[2] |= HIBYTE(id);
		map.tiles[x_c][y_c].data[3] &= 0x0003;
		map.tiles[x_c][y_c].data[3] |= LOBYTE(id);
	}
}

/** Return tile ID */
unsigned short get_tile(short x_c, short y_c, char layer) {
	unsigned short id;
	if(layer==0) {
        id = MAKEWORD(map.tiles[x_c][y_c].data[1] & 0x00c0, map.tiles[x_c][y_c].data[0]);
        id >>= 6;
	} else if(layer==1) {
		id = MAKEWORD(map.tiles[x_c][y_c].data[2] & 0x00f0, map.tiles[x_c][y_c].data[1] & 0x003f);
		id >>= 4;
	} else if(layer==2) {
		id = MAKEWORD(map.tiles[x_c][y_c].data[3] & 0x00fc, map.tiles[x_c][y_c].data[2] & 0x000f);
		id >>=2;
	}
    return id;
}
