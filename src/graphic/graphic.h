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

#ifndef _GRAPHIC_H
#define _GRAPHIC_H

class Graphics;

#include "../gui/npc.h"
#include "../gui/npc_text.h"
#include "../gui/buy.h"
#include "../gui/sell.h"
#include "../gui/buysell.h"
#include "../gui/chat.h"
#include "../gui/inventory.h"
#include "../gui/shop.h"
#include "../gui/chat.h"
#include "../gui/inventory.h"
#include "../gui/status.h"
#include "../gui/stats.h"
#include "../gui/skill.h"
#include "../gui/setup.h"
#include "../gui/equipment.h"
#include "../resources/resourcemanager.h"
#include "../map.h"
#include "spriteset.h"
#include <SDL.h>
#include <guichan/sdl.hpp>
#ifdef USE_OPENGL
#include <guichan/opengl.hpp>
#endif

#define TILE_SIZE 32

extern SDL_Surface *screen;
extern char speech[255];
extern char npc_text[1000];
extern char skill_points[10];
extern ChatBox *chatBox;
extern bool show_skill_dialog, show_skill_list_dialog;
extern int show_npc_dialog;
extern int map_x, map_y, camera_x, camera_y;
extern char npc_button[10];

extern StatusWindow *statusWindow;
extern BuyDialog *buyDialog;
extern SellDialog *sellDialog;
extern BuySellDialog *buySellDialog;
extern InventoryWindow *inventoryWindow;
extern NpcListDialog *npcListDialog;
extern NpcTextDialog *npcTextDialog;
extern SkillDialog *skillDialog;
extern StatsWindow *statsWindow;
extern Setup *setupWindow;
extern EquipmentWindow *equipmentWindow;

char get_x_offset(char, char);
char get_y_offset(char, char);

/**
 * The action listener for the chat field.
 */
class ChatListener : public gcn::ActionListener {
    public:
        void action(const std::string& eventId);
};

/**
 * The action listener for the buy or sell dialog.
 */
class BuySellListener : public gcn::ActionListener {
    public:
        void action(const std::string& eventId);
};

/**
 * 9 images defining a rectangle. 4 corners, 4 sides and a middle area. The
 * topology is as follows:
 *
 * <pre>
 *  !-----!-----------------!-----!
 *  !  0  !        1        !  2  !
 *  !-----!-----------------!-----!
 *  !  3  !        4        !  5  !
 *  !-----!-----------------!-----!
 *  !  6  !        7        !  8  !
 *  !-----!-----------------!-----!
 * </pre>
 *
 * Sections 0, 2, 6 and 8 will remain as is. 1, 3, 4, 5 and 7 will be
 * repeated to fit the size of the widget.
 */
struct ImageRect {
    Image *grid[9];
};

/**
 * A central point of control for graphics.
 */
#ifdef USE_OPENGL
class Graphics : public gcn::OpenGLGraphics {
#else
class Graphics : public gcn::SDLGraphics {
#endif
    public:
        /**
         * Constructor.
         */
        Graphics();

        /**
         * Destructor.
         */
        ~Graphics();

        /**
         * Draws a rectangle using images. 4 corner images, 4 side images and 1
         * image for the inside.
         */
        void drawImageRect(
                int x, int y, int w, int h,
                Image *topLeft, Image *topRight,
                Image *bottomLeft, Image *bottomRight,
                Image *top, Image *right,
                Image *bottom, Image *left,
                Image *center);

        /**
         * Draws a rectangle using images. 4 corner images, 4 side images and 1
         * image for the inside.
         */
        void drawImageRect(
                int x, int y, int w, int h,
                const ImageRect &imgRect);

        /**
         * Updates the screen. This is done by either copying the buffer to the
         * screen or swapping pages.
         */
        void updateScreen();

    private:
        Image *mouseCursor;
};

/**
 * Game engine that does the main drawing.
 */
class Engine {
    private:
        Spriteset *emotionset, *npcset, *monsterset;

    public:
        Engine();
        ~Engine();

        void draw();
};

#endif
