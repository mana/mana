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

#ifndef _INVENTORY_H
#define _INVENTORY_H 
#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif
#include "../main.h"
#include "../game.h"
#include "../log.h"
#include "../net/network.h"
#include "gui.h"

struct itemHolder { // the holder of a item
	int flag;		 // don't really know why I use this :)
	int itemIDNum;  // the id of the item
	int num;	    // number of items
	int xpos,ypos;  // where am I?
};

struct itemID {//the holder of the pictures for each item, maybe more in the future
	BITMAP * pic;
};

class TmwInventory{
	public:
		TmwInventory() {};
		~TmwInventory() {};

		void create(int tempxpos, int tempypos); // create the window
		void draw(BITMAP *); // draw the window (if areDisplaying != 0 )
		void show(int val); // choose between the show and don't show the window
		void toggle() { if(areDisplaying){show(0);}else{show(1);} }
		//API
		int addItem(int idnum, int antal); //add a item
		int rmItem(int idnum); //remove a item
		int changeNum(int idnum, int antal); // change nummber of a item
		//END API
	private:
		BITMAP * backgroundSmall;
		BITMAP * backgroundBig;
		BITMAP * title;
		BITMAP * empty;
		BITMAP * selected;
		itemHolder items[10][10]; // this is the test holder of items
		itemID     itemPIC[2]; // I only got two items
		int			ghostX, ghostY, ghostID, ghostOldIDX,ghostOldIDY; //info needed when moving item
		int dragingItem, lastSelectedX,lastSelectedY; //info needed when moving item
		int areDisplaying, dragingWindow;
		int bigwindow;
		int xpos, ypos; // Where am I ?
};

#endif
