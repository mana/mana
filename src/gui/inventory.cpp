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

		By SimEdw

*/

#include "inventory.h"

DIALOG inventory_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,   300,  252,  322,   60,    0,  -1,    0,    0,          0,                      0,    (char*)"Inventory",         NULL, NULL  },
   /*{ tmw_text_proc,     304,  284,  50,    10,    0,   0,    0,    0,          0,                      0,    (char*)"Name:",         NULL, NULL  },
   { tmw_text_proc,     304,  304,  50,    10,    0,   0,    0,    0,          0,                      0,(char*)"Password:",         NULL, NULL  },
   { tmw_edit_proc,     360,  280,  130,   18,    0,  -1,    0,    0,          24,                     0,          username,         NULL, NULL  },
   { tmw_password_proc, 360,  300,  130,   18,    0,  -1,    0,    0,          24,                     0,          password,         NULL, NULL  },
   { tmw_button_proc,   398,  322,  44,    18,    0,  -1,    'o',  D_EXIT,    -1,                      0,      (char*)"&Ok",         NULL, NULL  },
   { tmw_button_proc,   446,  322,  44,    18,    0,  -1,    'c',  D_EXIT,    -1,                      0,  (char*)"&Cancel",         NULL, NULL  },
   { tmw_check_proc,    304,  322,  60,    18,    0,   0,    '1',  0,          0,                      0,     (char*)"keep",         NULL, NULL  }, */
   { NULL,                0,    0,   0,     0,    0,   0,    0,    0,          0,                      0,              NULL,         NULL, NULL  },
};

DIALOG_PLAYER *inventory_player;

void TmwInventory::create(int tempxpos, int tempypos) {
	xpos = tempxpos;
	ypos = tempypos;
	itemset = load_datafile("./data/graphic/items.dat");
	empty = load_bitmap("./data/graphic/empty.bmp", NULL);
	selected = load_bitmap("./data/graphic/selected.bmp", NULL);

	for(int i = 0; i< 10; i++) {
		for(int ii = 0; ii< 10; ii++) {
			items[i][ii].flag = 0; //doesn't hold anything
			items[i][ii].itemIDNum = -1; //doesn't exist :)
			items[i][ii].xpos = empty->w*i+1;
			items[i][ii].ypos = empty->h*ii+20;
			items[i][ii].num = 0;
		}
	}
	//draw_rle_sprite(buffer, (RLE_SPRITE *)itemPIC[items[itemX][itemY].itemIDNum].pic, (xpos+items[itemX][itemY].xpos), (ypos+items[itemX][itemY].ypos));

	//create two fake items
	/*items[0][0].flag = 1;
	items[0][0].itemIDNum = 0;
	items[0][0].num = 1;

	items[2][0].flag = 1;
	items[2][0].itemIDNum = 1;
	items[2][0].num = 3;*/

	backgroundSmall = create_bitmap(empty->w*10+10, empty->h+10);
	backgroundBig = create_bitmap(empty->w*10+10, empty->h*10+10);
	title = create_bitmap(15, backgroundSmall->h);
	clear_to_color(title, makecol(0, 0, 200));
	clear_to_color(backgroundSmall, makecol(0,0,100));
	clear_to_color(backgroundBig, makecol(0,0,100));
	areDisplaying = 0;
	dragingWindow = 0;
	lastSelectedX = -1;
	lastSelectedY = -1;
	bigwindow = 0; //false 

  inventory_player = init_dialog(inventory_dialog, -1);

}

void TmwInventory::draw(BITMAP * buffer) {
	// let's draw the inventory
	if(areDisplaying) {
		position_dialog(inventory_dialog, xpos, ypos);
		dialog_message(inventory_dialog,MSG_DRAW,0,0);
		update_dialog(inventory_player);
		int max = 1;
		/*if(!bigwindow) {
			max = 1;
			blit(backgroundSmall, buffer, 0, 0, xpos-5, ypos-5, 800, 600);
		} else {
			max = 10;
			blit(backgroundBig, buffer, 0, 0, xpos-5, ypos-5, 800, 600);
		}*/
		//blit(title, buffer, 0, 0, xpos+backgroundSmall->w-5, ypos+-5, 800, 600);
		for(int itemX=0;itemX<10;itemX++) {
			for(int itemY=0;itemY<max;itemY++) {
				int draw = 0;
				//blit(empty, buffer, 0, 0, (xpos+items[itemX][itemY].xpos), (ypos+items[itemX][itemY].ypos), 800, 600);
	
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
        } else { // if !mouse_b&1
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

        if(mouse_b&2 && items[itemX][itemY].flag) { // if roght mouse button over an item
					if(xpos+items[itemX][itemY].xpos+empty->w > mouse_x && xpos+items[itemX][itemY].xpos < mouse_x)
						if(ypos+items[itemX][itemY].ypos+empty->h > mouse_y && ypos+items[itemX][itemY].ypos < mouse_y) {
						//selected
              masked_blit(selected, buffer, 0, 0, (xpos+items[itemX][itemY].xpos), (ypos+items[itemX][itemY].ypos), 800, 600);
              draw = 1;
              if(itemMeny){ itemMeny=0; } else { itemMeny=1; itemIdn =items[itemX][itemY].itemIDNum ;itemMeny_x = (xpos+items[itemX][itemY].xpos)+selected->w;itemMeny_y = (ypos+items[itemX][itemY].ypos)+selected->h;}
						}
				}

        if(xpos+items[itemX][itemY].xpos+empty->w > mouse_x && xpos+items[itemX][itemY].xpos < mouse_x && ypos+items[itemX][itemY].ypos+empty->h > mouse_y && ypos+items[itemX][itemY].ypos < mouse_y ) {
          // a hoover
          lastSelectedX = itemX;
          lastSelectedY = itemY;
        }

				xpos = inventory_dialog[0].x;
        ypos = inventory_dialog[0].y;

        if(items[itemX][itemY].flag) //draw the item
					if(items[itemX][itemY].itemIDNum>=501 && items[itemX][itemY].itemIDNum<=510)
            masked_blit((BITMAP *)itemset[items[itemX][itemY].itemIDNum-501].dat, buffer, 0, 0, (xpos+items[itemX][itemY].xpos), (ypos+items[itemX][itemY].ypos), 32, 32);
					else
						masked_blit((BITMAP *)itemset[0].dat, buffer, 0, 0, (xpos+items[itemX][itemY].xpos), (ypos+items[itemX][itemY].ypos), 32, 32);

        //the number of that item
        if(!bigwindow)
          alfont_textprintf_centre_aa(buffer, gui_font, xpos+items[itemX][itemY].xpos+20, ypos+items[itemX][itemY].ypos+empty->h-15, makecol(0,0,0), "%i",items[itemX][itemY].num);
        else
          alfont_textprintf_centre_aa(buffer, gui_font, xpos+items[itemX][itemY].xpos+20, ypos+items[itemX][itemY].ypos+empty->h-15, makecol(0,0,0), "%i",items[itemX][itemY].num);

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

	/*if(mouse_b&1) {
		if(xpos+title->w+backgroundSmall->w > mouse_x && xpos+backgroundSmall->w < mouse_x)
			if(ypos+title->h > mouse_y && ypos < mouse_y) { //begin to move the window
				xpos = mouse_x-(backgroundSmall->w);
				ypos = mouse_y;
				dragingWindow=1;
			}
	} else { dragingWindow=0;}*/
				
	/*if(dragingWindow) { //moving the window ?
		xpos = mouse_x-(backgroundSmall->w);
		ypos = mouse_y;
	}*/
			
	if(dragingItem) { //moving the item
 		masked_blit((BITMAP *)itemset[ghostID].dat, buffer, 0, 0, ghostX, ghostY, 800, 600);
		ghostX = mouse_x;
		ghostY = mouse_y;
	}
	
	if(itemMeny){
    if(itemMeny_y < mouse_y && itemMeny_y+10 > mouse_y) {
      if(mouse_b&1) {
        useItem(itemIdn);
        itemMeny = 0;
			}
      alfont_textprintf_aa(buffer, gui_font, itemMeny_x, itemMeny_y, makecol(255,237,33), "Use item");
    } else {
      alfont_textprintf_aa(buffer, gui_font, itemMeny_x, itemMeny_y, MAKECOL_BLACK, "Use item");
		}
    if(itemMeny_y+10 < mouse_y && itemMeny_y+20 > mouse_y) {
      if(mouse_b&1) {
        rmItem(itemIdn);
        itemMeny = 0;
			}
      alfont_textprintf_aa(buffer, gui_font, itemMeny_x, itemMeny_y+10, makecol(255,237,33), "Del item");
    } else {
      alfont_textprintf_aa(buffer, gui_font, itemMeny_x, itemMeny_y+10, MAKECOL_BLACK, "Del item");
		}
	}
}


void TmwInventory::show(int val) {
	if(val)
		areDisplaying = 1;

	if(!val)
		areDisplaying = 0; 
}

int TmwInventory::addItem(int idnum, int antal) {
	int found, tempi, tempii = 0;
	found = 0;
	tempi = -1;
	for(int i = 0; i< 10; i++) {
		for(int ii = 0; ii< 10; ii++) {
				if(items[i][ii].itemIDNum == idnum) {
					found = 1; items[i][ii].num = antal;
					return -2;
				}
			}
		}

	if(!found) {
		for(int ii = 0; ii< 10; ii++) {
			for(int i = 0; i< 10; i++) {
				if(items[i][ii].flag == 0) {
					tempi = i;
					tempii = ii;
					ii=10;
					i=10;
				}
			}
		}

		if(tempi != -1) {
			items[tempi][tempii].flag = 1;
			items[tempi][tempii].itemIDNum = idnum;
			items[tempi][tempii].num = antal;
			return 1;
		} else {
			return -1;
		}
	}
	return -3;
}

int TmwInventory::rmItem(int idnum) {
  int found, tempi;
  found = 0;
  tempi = -1;
  for(int i = 0; i< 10; i++) {
    for(int ii = 0; ii< 10; ii++) {
      if(items[i][ii].itemIDNum == idnum) {
				items[i][ii].itemIDNum = -1;
        items[i][ii].flag = 0;
        items[i][ii].num = 0;
        return 1;
      }
    }
	}
  return -1;
}

int TmwInventory::changeNum(int idnum, int antal) {
  int found, tempi;
  found = 0;
  tempi = -1;
	for(int i = 0; i< 10; i++) {
    for(int ii = 0; ii< 10; ii++) {
      if(items[i][ii].itemIDNum == idnum) {
				items[i][ii].num = antal;
				return 1;
			}
    }
  }
  return -1;
}

int TmwInventory::useItem(int idnum) {
	printf("Use item %i\n",idnum);
	WFIFOW(0) = net_w_value(0x00a7);
	WFIFOW(2) = net_w_value(idnum);
	WFIFOSET(4);
	while((out_size>0))flush();

	return 0;
}
