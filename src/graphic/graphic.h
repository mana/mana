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

#ifndef _GRAPHIC_H
#define _GRAPHIC_H

#include "../game.h"
#include "../map.h"
#include "../being.h"
#include "../gui/buy.h"
#include "../gui/npc.h"
#include "../gui/npc_text.h"
#include "../gui/sell.h"
#include "../gui/buysell.h"
#include "../gui/chat.h"
#include "../gui/inventory.h"
#include "../gui/shop.h"
#include "../gui/chat.h"
#include "../gui/inventory.h"
#include "../gui/npc.h"
#include "../gui/status.h"
#include "../../data/graphic/gfx_data.h"
#include "image.h"
#include <allegro.h>

#define TILE_SIZE 32

extern BITMAP *buffer;
extern int page_num;
extern char speech[255];
extern char npc_text[1000];
extern char skill_points[10];
extern Chat chatlog;
extern bool show_skill_dialog, show_skill_list_dialog;
extern int show_npc_dialog;
extern int map_x, map_y, camera_x, camera_y;
extern BITMAP *hairset;
extern char npc_button[10];

extern StatusWindow *statusWindow;
extern BuyDialog *buyDialog;
extern SellDialog *sellDialog;
extern BuySellDialog *buySellDialog;
extern InventoryWindow *inventoryWindow;
extern NpcListDialog *npcListDialog;
extern NpcTextDialog *npcTextDialog;

// The action listener for the chat field
class ChatListener : public gcn::ActionListener {
    public:
        void action(const std::string& eventId);
};

// The action listener for the buy or sell dialog
class BuySellListener : public gcn::ActionListener {
    public:
        void action(const std::string& eventId);
};

class GraphicEngine {
    private:
        Spriteset *tileset, *emotionset, *npcset, *playerset, *monsterset, *hairset;
        BITMAP *buffer;
        
    public:
        GraphicEngine();
        ~GraphicEngine();
        void refresh();	
};

#endif
