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

#ifdef WIN32
  #pragma warning (disable:4312)
#endif
#include "login.h"
#ifndef _TMW_GUI
#define _TMW_GUI

#include <allegro.h>
#include <alfont.h>
#include <string.h>
#include <guichan.hpp>
#include <guichan/allegro.hpp>

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
extern ALFONT_FONT *gui_font;

extern gcn::Container* guiTop;                // The top container
extern gcn::AllegroGraphics* guiGraphics;     // Graphics driver

/* Definition of the callback function prototypes */
typedef int (*gui_buttonCallback)(int id);
typedef char *(*getfuncptr)(int, int *);

void init_gui(BITMAP *dest_bitmap, const char *skin);
void gui_exit();
int  gui_update(DIALOG_PLAYER *player);
int  gui_load_skin(const char* skinname);
void gui_shutdown(void);

int tmw_button_proc(int msg, DIALOG *d, int c);
int tmw_slider_proc(int msg, DIALOG *d, int c);
int tmw_check_proc(int msg, DIALOG *d, int c);
int tmw_radio_proc(int msg, DIALOG *d, int c);
int tmw_edit_proc(int msg, DIALOG *d, int c);
int tmw_password_proc(int msg, DIALOG *d, int c);
int tmw_list_proc(int msg, DIALOG *d, int c);
int tmw_text_proc(int msg, DIALOG *d, int c);
int tmw_dialog_proc(int msg, DIALOG *d, int c);
int tmw_ldialog_proc(int msg, DIALOG *d, int c);
int tmw_textbox_proc(int msg, DIALOG *d, int c);
int tmw_bitmap_proc(int msg, DIALOG *d, int c);
int tmw_bar_proc(int msg, DIALOG *d, int c);
int tmw_plus_proc(int msg, DIALOG *d, int c);

void ok(const char *title, const char *message);
unsigned int yes_no(const char *title, const char *message);

#endif
