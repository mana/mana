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

#ifndef _ENGINE_H
#define _ENGINE_H

#include "gui/npc.h"
#include "gui/npc_text.h"
#include "gui/buy.h"
#include "gui/sell.h"
#include "gui/buysell.h"
#include "gui/buddywindow.h"
#include "gui/chat.h"
#include "gui/inventory.h"
#include "gui/shop.h"
#include "gui/inventory.h"
#include "gui/status.h"
#include "gui/stats.h"
#include "gui/skill.h"
#include "gui/newskill.h"
#include "gui/setup.h"
#include "gui/equipment.h"
#include "gui/chargedialog.h"
#include "gui/trade.h"
#include "gui/help.h"
#include "gui/popupmenu.h"
#include "resources/resourcemanager.h"
#include "map.h"
#include "graphic/spriteset.h"

extern char speech[255];
extern char npc_text[1000];
extern char skill_points[10];
extern bool show_skill_dialog, show_skill_list_dialog;
extern int show_npc_dialog;
extern int map_x, map_y, camera_x, camera_y;
extern char npc_button[10];

extern ChatWindow *chatWindow;
extern StatusWindow *statusWindow;
extern BuyDialog *buyDialog;
extern SellDialog *sellDialog;
extern BuySellDialog *buySellDialog;
extern InventoryWindow *inventoryWindow;
extern NpcListDialog *npcListDialog;
extern NpcTextDialog *npcTextDialog;
extern SkillDialog *skillDialog;
extern NewSkillDialog *newSkillWindow;
extern StatsWindow *statsWindow;
extern Setup *setupWindow;
extern EquipmentWindow *equipmentWindow;
extern ChargeDialog* chargeDialog;
extern TradeWindow *tradeWindow;
extern BuddyWindow *buddyWindow;
extern HelpWindow *helpWindow;
extern PopupMenu *popupMenu;
extern std::map<int, Spriteset*> monsterset;
char get_x_offset(char, char);
char get_y_offset(char, char);

/**
 * Game engine that does the main drawing.
 */
class Engine
{
    public:
        /**
         * Constructor.
         */
        Engine();

        /**
         * Destructor.
         */
        ~Engine();

        /**
         * Returns the currently active map.
         */
        Map *getCurrentMap();

        /**
         * Sets the currently active map.
         */
        void setCurrentMap(Map *newMap);

        /**
         * Performs engine logic.
         */
        void logic();

        /**
         * Draws everything on the screen.
         */
        void draw();

    private:
        Spriteset *emotionset, *npcset, *weaponset, *itemset;
        Map *mCurrentMap;
};

#endif
