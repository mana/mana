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

#include "inventory.h"

void TmwInventory::create(int tempxpos, int tempypos) {
	xpos =tempxpos;
	ypos =tempypos;

	itemPIC[0].pic = load_bitmap("items/herb.bmp", NULL); //ID=0
	itemPIC[1].pic = load_bitmap("items/magicthing.bmp", NULL); //ID=1
	empty = load_bitmap("items/empty.bmp", NULL);
	selected = load_bitmap("items/selected.bmp", NULL);

	for(int i = 0; i< 10; i++) {
		for(int ii = 0; ii< 10; ii++) {
			items[i][ii].flag = 0; //doesn't hold anything
			items[i][ii].itemIDNum = -1; //doesn't exist :)
			items[i][ii].xpos = empty->w*i+1;
			items[i][ii].ypos = empty->h*ii;
			items[i][ii].num = 0;
		}
	}

	//create two fake items
	items[0][0].flag = 1;
	items[0][0].itemIDNum = 0;
	items[0][0].num = 1;

	items[2][0].flag = 1;
	items[2][0].itemIDNum = 1;
	items[2][0].num = 3;

	backgroundSmall = create_bitmap(empty->w*10+10, empty->h+10);
	backgroundBig = create_bitmap(empty->w*10+10, empty->h*10+10);
	title = create_bitmap(15, backgroundSmall->h);
	floodfill(title,0,0,200);
	floodfill(backgroundSmall,0,0,100);
	floodfill(backgroundBig,0,0,100);
	areDisplaying = 0;
	dragingWindow = 0;
	lastSelectedX = -1;
	lastSelectedY = -1;
	bigwindow = 0; //false 
}

void TmwInventory::draw(BITMAP * buffer) {
	// let's draw the inventory
	if(areDisplaying) {
		int max;
		if(!bigwindow) {
			max = 1;
			blit(backgroundSmall, buffer, 0, 0, xpos-5, ypos-5, 800, 600);
		} else {
			max = 10;
			blit(backgroundBig, buffer, 0, 0, xpos-5, ypos-5, 800, 600);
		}
		blit(title, buffer, 0, 0, xpos+backgroundSmall->w-5, ypos+-5, 800, 600);
		for(int itemX = 0; itemX< 10; itemX++) {
			for(int itemY = 0;itemY<max;itemY++) {
				int draw = 0;
	
				blit(empty, buffer, 0, 0, (xpos+items[itemX][itemY].xpos), (ypos+items[itemX][itemY].ypos), 800, 600);
	
				if(mouse_b&1) {
					if(xpos+items[itemX][itemY].xpos+empty->w > mouse_x && xpos+items[itemX][itemY].xpos < mouse_x)
						if(ypos+items[itemX][itemY].ypos+empty->h > mouse_y && ypos+items[itemX][itemY].ypos < mouse_y) {
						//selected
						masked_blit(selected, buffer, 0, 0, (xpos+items[itemX][itemY].xpos), (ypos+items[itemX][itemY].ypos), 800, 600);
						draw = 1;
						if(items[itemX][itemY].flag) // have a item
							if(!dragingItem) { //not dragging it
								dragingItem=1; //begin to drag
								ghostOldIDX = itemX;
								ghostOldIDY = itemY; 
								ghostID = items[itemX][itemY].itemIDNum;
								ghostX = mouse_x;
								ghostY = mouse_y;
							}
						}
					} else {
						if(lastSelectedX != -1 && dragingItem) { // have stoped dragging it over a itemholder
							//swap place
							itemHolder temp;
							int txpos1,typos1,txpos2,typos2;
							txpos1 = items[lastSelectedX][lastSelectedY].xpos;
							typos1 = items[lastSelectedX][lastSelectedY].ypos;
							txpos2 = items[ghostOldIDX][ghostOldIDY].xpos;
							typos2 = items[ghostOldIDX][ghostOldIDY].ypos;
							temp = items[lastSelectedX][lastSelectedY];
							items[lastSelectedX][lastSelectedY] = items[ghostOldIDX][ghostOldIDY];
							items[ghostOldIDX][ghostOldIDY] = temp;
							items[lastSelectedX][lastSelectedY].xpos = txpos1;
							items[lastSelectedX][lastSelectedY].ypos = typos1;
							items[ghostOldIDX][ghostOldIDY].xpos = txpos2;
							items[ghostOldIDX][ghostOldIDY].ypos = typos2;
						}
						dragingItem = 0; // stop dragging
					}
	
					if(xpos+items[itemX][itemY].xpos+empty->w > mouse_x && xpos+items[itemX][itemY].xpos < mouse_x && ypos+items[itemX][itemY].ypos+empty->h > mouse_y && ypos+items[itemX][itemY].ypos < mouse_y ) {
							// a hoover
							lastSelectedX = itemX;
							lastSelectedY = itemY;
					}
		
					if(items[itemX][itemY].flag) //draw the item
						masked_blit(itemPIC[items[itemX][itemY].itemIDNum].pic, buffer, 0, 0, (xpos+items[itemX][itemY].xpos), (ypos+items[itemX][itemY].ypos), 800, 600);
		
					//the number of that item
					if(!bigwindow)
						alfont_textprintf_aa(buffer, gui_font, xpos+items[itemX][itemY].xpos+20, ypos+items[itemX][itemY].ypos+empty->h-15, makecol(0,0,0), "%i",items[itemX][itemY].num);
					else
						alfont_textprintf_aa(buffer, gui_font, xpos+items[itemX][itemY].xpos+20, ypos+items[itemX][itemY].ypos+empty->h-15, makecol(0,0,0), "%i",items[itemX][itemY].num);
				}
			}
	
		
		if(mouse_b&2)	{
			if(xpos+title->w+backgroundSmall->w > mouse_x && xpos+backgroundSmall->w < mouse_x)
				if(ypos+title->h > mouse_y && ypos < mouse_y) {
					if(bigwindow)
						bigwindow=0;
					else
						bigwindow = 1;
				}
			}
	}

	if(mouse_b&1) {
		if(xpos+title->w+backgroundSmall->w > mouse_x && xpos+backgroundSmall->w < mouse_x)
			if(ypos+title->h > mouse_y && ypos < mouse_y) { //begin to move the window
				xpos = mouse_x-(backgroundSmall->w);
				ypos = mouse_y;
				dragingWindow=1;
			}
	} else { dragingWindow=0;}
				
	if(dragingWindow) { //moving the window ?
		xpos = mouse_x-(backgroundSmall->w);
		ypos = mouse_y;
	}
			
	if(dragingItem) { //moving the item
 		masked_blit(itemPIC[ghostID].pic, buffer, 0, 0, ghostX, ghostY, 800, 600);
		ghostX = mouse_x;
		ghostY = mouse_y;
	}
}


void TmwInventory::show(int val) {
	if(val)
		areDisplaying = 1;

	if(!val)
		areDisplaying = 0; 
}

int TmwInventory::addItem(int idnum, int antal)
{
int found, tempi, tempii;
found = 0;
tempi = -1;
for(int i = 0; i< 10; i++)
	{
	for(int ii = 0; ii< 10; ii++)
		{
		if(items[i][ii].itemIDNum == idnum)
			{ found = 1; items[i][ii].num = antal; return -2; }
		}
	}
if(!found)
{
for(int ii = 0; ii< 10; ii++)
	{
	for(int i = 0; i< 10; i++)
		{
	if(items[i][ii].flag == 0)
		{tempi = i; tempii = ii; ii=10;i=10;}
		}
	}
	if(tempi != -1)
	{
	items[tempi][tempii].flag = 1;
	items[tempi][tempii].itemIDNum = idnum;
	items[tempi][tempii].num = antal;
	return 1;
	} else { return -1; }
	
}
return -3;
}

int TmwInventory::rmItem(int idnum)
{
int found, tempi;
found = 0;
tempi = -1;
for(int i = 0; i< 10; i++)
	{
	for(int ii = 0; ii< 10; ii++)
		{
		if(items[i][ii].itemIDNum == idnum)
			{ items[i][ii].itemIDNum = -1;
			items[i][ii].flag = 0;
			items[i][ii].num = 0;
			return 1;
			}
		}
	}
return -1;
}

int TmwInventory::changeNum(int idnum, int antal)
{
int found, tempi;
found = 0;
tempi = -1;
for(int i = 0; i< 10; i++)
	{
	for(int ii = 0; ii< 10; ii++)
		{
		if(items[i][ii].itemIDNum == idnum)
			{ items[i][ii].num = antal; return 1; }
		}
	}
return -1;
}