/**

	The Mana World
	Copyright 2004 The Mana World Development Team

    This file is part of The Mana World.

    The Mana World is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    The Mana World is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Mana World; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifdef WIN32
  #pragma warning (disable:4312)
#endif

#ifndef _SUPER_EAGLE_H
#define _SUPER_EAGLE_H

#include <string.h>
#include <allegro.h>
#include <allegro/internal/aintern.h>

#define uint32 unsigned long
#define uint16 unsigned short
#define uint8 unsigned char

void SuperEagle(BITMAP * src, BITMAP * dest, int s_x, int s_y, int d_x, int d_y, int w, int h);
void SuperEagle_ex(uint8 *src, uint32 src_pitch, uint8 *unused, BITMAP *dest, uint32 width, uint32 height);

int Init_SuperEagle(int d);

#endif
