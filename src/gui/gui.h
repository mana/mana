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

#ifndef _TMW_GUI
#define _TMW_GUI

#ifdef WIN32
  #pragma warning (disable:4312)
#endif

#include <guichan.hpp>
#include <guichan/allegro.hpp>
#include <allegro.h>
#include <string.h>

/**
 * \defgroup GUI GUI related classes
 */

typedef struct {
    BITMAP *grid[9];
} LexSkinnedRect;

typedef struct {
    LexSkinnedRect  background[4];
    int             textcolor[4];
} LexButton;

typedef struct {
    BITMAP          *hSlider[3];
    BITMAP          *vSlider[3];
    BITMAP          *hGrip;
    BITMAP          *vGrip;
} LexSlider;

typedef struct {
    BITMAP *normal;
    BITMAP *checked;
    BITMAP *disabled;
    BITMAP *disabled_checked;
    int     textcolor[2];
} LexCheckbox;

typedef struct {
    BITMAP *normal;
    BITMAP *checked;
    BITMAP *disabled;
    BITMAP *disabled_checked;
    int     textcolor[2];
} LexRadiobutton;

typedef struct {
    LexSkinnedRect bg;
    int            textcolor[2];
} LexTextbox;

typedef struct {
    LexSkinnedRect bg;
    LexSkinnedRect vscroll;
    int            textcolor[4];
} LexListbox;

typedef struct {
    LexSkinnedRect bg;
} LexDialog;

typedef struct {
    LexSkinnedRect bg;
} LexBar;

typedef struct {
    LexSkinnedRect bg;
} LexPlus;

typedef struct {
    LexButton      button;
    LexSlider      slider;
    LexCheckbox    checkbox;
    LexRadiobutton radiobutton;
    LexTextbox     textbox;
    LexListbox     listbox;
    LexDialog      dialog;
    LexBar		   bar;
    LexPlus		   plus;
} LexSkin;

extern LexSkin gui_skin;
extern BITMAP *gui_bitmap;

extern gcn::Container* guiTop;                // The top container
extern gcn::AllegroGraphics* guiGraphics;     // Graphics driver
extern gcn::ImageFont* guiFont;               // The gui font

void init_gui(BITMAP *dest_bitmap, const char *skin);
void gui_exit();
int gui_update(DIALOG_PLAYER *player);
int gui_load_skin(const char* skinname);
void gui_shutdown(void);

/** Helper procedure to draw skinned rectangles */
void draw_skinned_rect(BITMAP*dst, LexSkinnedRect *skin,
        int x, int y, int w, int h);

/** Draw text for gui widgets */
int gui_text(BITMAP *bmp, AL_CONST char *s,
        int x, int y, int color, int centre);

// Old Allegro GUI procs
int tmw_button_proc(int msg, DIALOG *d, int c);
int tmw_text_proc(int msg, DIALOG *d, int c);
int tmw_dialog_proc(int msg, DIALOG *d, int c);

// Last two remaining Allegro GUI dialogs
void ok(const char *title, const char *message);
unsigned int yes_no(const char *title, const char *message);

#endif
