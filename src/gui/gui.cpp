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

#include "gui.h"
#include <allegro.h>
#include <allegro/internal/aintern.h>
#include <math.h>
#include "../log.h"
#include "../sound/sound.h"
#include "allegroinput.h"

#ifndef WIN32
#define __cdecl __attribute__((cdecl))
#endif

#define GUI_BMP_COUNT 11
#define GUI_BMP_OFS_BUTTON 0
#define GUI_BMP_OFS_SLIDER 4
#define GUI_BMP_OFS_CHECKBOX 5
#define GUI_BMP_OFS_RADIOBUTTON 6
#define GUI_BMP_OFS_TEXTBOX 7
#define GUI_BMP_OFS_LISTBOX 8
#define GUI_BMP_OFS_DIALOG 9

#define GUI_CALL_BUTTONCALLBACK(d)
static BITMAP *gui__repository[GUI_BMP_COUNT];

/* The currently active skin */
LexSkin gui_skin;
BITMAP *gui_bitmap;
bool drag;
DATAFILE *gui_gfx;


/* very internal update stuff */
int (*gui__external_slider_callback)(void *, int);
int reroute_slider_proc(void *dp3, int d2);

// Guichan Allegro stuff
gcn::AllegroInput* guiInput;           // Input driver
gcn::AllegroGraphics* guiGraphics;     // Graphics driver
gcn::AllegroImageLoader* imageLoader;  // For loading images

// Guichan stuff
gcn::Gui* gui;            // A Gui object - binds it all together
gcn::Container* guiTop;   // The top container
gcn::ImageFont* guiFont;  // A font




/** Initialize gui system */
void init_gui(BITMAP *bitmap, const char *skin) {
    imageLoader = new gcn::AllegroImageLoader();
    gcn::Image::setImageLoader(imageLoader);

    guiGraphics = new gcn::AllegroGraphics();
    guiGraphics->setTarget(bitmap);

    guiInput = new AllegroInput();

    guiTop = new gcn::Container();
    guiTop->setDimension(gcn::Rectangle(0, 0, SCREEN_W, SCREEN_H));
    guiTop->setOpaque(false);

    gui = new gcn::Gui();
    gui->setGraphics(guiGraphics);
    gui->setInput(guiInput);
    gui->setTop(guiTop);
    guiFont = new gcn::ImageFont("./data/graphic/fixedfont.bmp",
            " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:@"
            "!\"$%&/=?^+*#[]{}()<>_;'.,\\|-~`"
            );
    gcn::Widget::setGlobalFont(guiFont);


    gui_bitmap = bitmap;
    gui_load_skin(skin);
    drag = false;
    show_mouse(NULL);
}

int gui_update(DIALOG_PLAYER *player) {
    int ret = 0;

    if (player) {
        // Update Allegro dialog (to be replaced)
        dialog_message(player->dialog, MSG_DRAW, 0, 0);
        ret = update_dialog(player);
    }
    else {
        // Update new GUI system using Guichan
        gui->logic();
        gui->draw();
    }

    // Draw the mouse
    draw_sprite(gui_bitmap, mouse_sprite, mouse_x, mouse_y);

    return ret;
}


void loadButtonSkin() {
    char **tokens;
    int    tokenCount;
    int    gridx[4];
    int    gridy[4];
    int    a = 0;
    int    x,y,mode;

    tokens = get_config_argv("button", "gridx", &tokenCount);
    for (a=0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("button", "gridy", &tokenCount);
    for (a=0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }

    tokens                      = get_config_argv("button", "textcol_norm", &tokenCount);
    gui_skin.button.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                      = get_config_argv("button", "textcol_hilite", &tokenCount);
    gui_skin.button.textcolor[1] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                      = get_config_argv("button", "textcol_pressed", &tokenCount);
    gui_skin.button.textcolor[2] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                      = get_config_argv("button", "textcol_disabled", &tokenCount);
    gui_skin.button.textcolor[3] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));

    gui__repository[GUI_BMP_OFS_BUTTON + 0] = (BITMAP *)gui_gfx[0].dat;
		gui__repository[GUI_BMP_OFS_BUTTON + 1] = (BITMAP *)gui_gfx[2].dat;
    gui__repository[GUI_BMP_OFS_BUTTON + 2] = (BITMAP *)gui_gfx[3].dat;
    gui__repository[GUI_BMP_OFS_BUTTON + 3] = (BITMAP *)gui_gfx[1].dat;

    for (mode=0; mode < 4; mode++) {
        a=0;
        for (y=0; y < 3; y++) {
            for (x=0; x < 3; x++) {
                gui_skin.button.background[mode].grid[a] = create_sub_bitmap(
                                gui__repository[GUI_BMP_OFS_BUTTON + mode],
                                gridx[x]             , gridy[y],
                                gridx[x+1]-gridx[x]+1, gridy[y+1]-gridy[y]+1
                                );
                a++;
            }
        }
    }
}


void loadSliderSkin() {
    int    x, y, w, h,o1,o2;
    char **tokens;
    int    tokenCount;

		gui__repository[GUI_BMP_OFS_SLIDER] = (BITMAP *)gui_gfx[8].dat;
		if(!gui__repository[GUI_BMP_OFS_SLIDER])alert("","","","","",0,0);

    tokens = get_config_argv("slider", "slider_h", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);

    tokens = get_config_argv("slider", "slider_h_ofs", &tokenCount);
    o1 = atoi(tokens[0]); o2 = atoi(tokens[1]);

    gui_skin.slider.hSlider[0] = create_sub_bitmap(gui__repository[GUI_BMP_OFS_SLIDER], x , y, o1-x    , h);
    gui_skin.slider.hSlider[1] = create_sub_bitmap(gui__repository[GUI_BMP_OFS_SLIDER], o1, y, o2-o1   , h);
    gui_skin.slider.hSlider[2] = create_sub_bitmap(gui__repository[GUI_BMP_OFS_SLIDER], o2, y, w-(o2-x), h);

    tokens = get_config_argv("slider", "slider_v", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);

    tokens = get_config_argv("slider", "slider_v_ofs", &tokenCount);
    o1 = atoi(tokens[0]); o2 = atoi(tokens[1]);

    gui_skin.slider.vSlider[0] = create_sub_bitmap(gui__repository[GUI_BMP_OFS_SLIDER], x,  y, w, o1 -     y);
    gui_skin.slider.vSlider[1] = create_sub_bitmap(gui__repository[GUI_BMP_OFS_SLIDER], x, o1, w, o2 -     o1);
    gui_skin.slider.vSlider[2] = create_sub_bitmap(gui__repository[GUI_BMP_OFS_SLIDER], x, o2, w,  h - (o2-y));

    tokens = get_config_argv("slider", "handle_v", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    gui_skin.slider.vGrip   = create_sub_bitmap(gui__repository[GUI_BMP_OFS_SLIDER], x, y, w, h);

    tokens = get_config_argv("slider", "handle_h", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    gui_skin.slider.hGrip   = create_sub_bitmap(gui__repository[GUI_BMP_OFS_SLIDER], x, y, w, h);
}

void loadCheckboxSkin()  {
    int    x, y, w,h;
    char **tokens;
    int    tokenCount;

		gui__repository[GUI_BMP_OFS_CHECKBOX] = (BITMAP *)gui_gfx[4].dat;


    tokens = get_config_argv("checkbox", "normal", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    gui_skin.checkbox.normal = create_sub_bitmap(gui__repository[GUI_BMP_OFS_CHECKBOX], x , y, w, h);

    tokens = get_config_argv("checkbox", "checked", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    gui_skin.checkbox.checked = create_sub_bitmap(gui__repository[GUI_BMP_OFS_CHECKBOX], x , y, w, h);

    tokens = get_config_argv("checkbox", "disabled", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    gui_skin.checkbox.disabled = create_sub_bitmap(gui__repository[GUI_BMP_OFS_CHECKBOX], x , y, w, h);

    tokens = get_config_argv("checkbox", "disabled_check", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    gui_skin.checkbox.disabled_checked = create_sub_bitmap(gui__repository[GUI_BMP_OFS_CHECKBOX], x , y, w, h);

    tokens                        = get_config_argv("button", "textcol_norm", &tokenCount);
    gui_skin.checkbox.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                        = get_config_argv("button", "textcol_disabled", &tokenCount);
    gui_skin.checkbox.textcolor[1] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
}

void loadTextboxSkin() {
    char **tokens;
    int    tokenCount;
    int    gridx[4];
    int    gridy[4];
    int    a = 0;
    int    x,y;

    tokens = get_config_argv("textbox", "gridx", &tokenCount);
    for (a=0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("textbox", "gridy", &tokenCount);
    for (a=0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }

    tokens                       = get_config_argv("textbox", "textcol_norm", &tokenCount);
    gui_skin.textbox.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                       = get_config_argv("textbox", "textcol_disabled", &tokenCount);
    gui_skin.textbox.textcolor[1] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));

		gui__repository[GUI_BMP_OFS_TEXTBOX] = (BITMAP *)gui_gfx[9].dat;



    a=0;
    for (y=0; y < 3; y++) {
        for (x=0; x < 3; x++) {
            gui_skin.textbox.bg.grid[a] = create_sub_bitmap(
                            gui__repository[GUI_BMP_OFS_TEXTBOX],
                            gridx[x]             , gridy[y],
                            gridx[x+1]-gridx[x]+1, gridy[y+1]-gridy[y]+1
                            );
            a++;
        }
    }
}

void loadListboxSkin() {
    char **tokens;
    int    tokenCount;
    int    gridx[4];
    int    gridy[4];
    int    a = 0;
    int    x,y;

    tokens = get_config_argv("listbox", "gridx", &tokenCount);
    for (a=0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("listbox", "gridy", &tokenCount);
    for (a=0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }

    tokens                       = get_config_argv("listbox", "textcol_norm", &tokenCount);
    gui_skin.listbox.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                       = get_config_argv("listbox", "textcol_selected", &tokenCount);
    gui_skin.listbox.textcolor[1] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                       = get_config_argv("listbox", "textbg_selected", &tokenCount);
    gui_skin.listbox.textcolor[2] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                       = get_config_argv("listbox", "textcol_disabled", &tokenCount);
    gui_skin.listbox.textcolor[3] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));

		gui__repository[GUI_BMP_OFS_LISTBOX+0] = (BITMAP *)gui_gfx[6].dat;
		gui__repository[GUI_BMP_OFS_LISTBOX+1] = (BITMAP *)gui_gfx[10].dat;

    a=0;
    for (y=0; y < 3; y++) {
        for (x=0; x < 3; x++) {
            gui_skin.listbox.bg.grid[a] = create_sub_bitmap(
                            gui__repository[GUI_BMP_OFS_LISTBOX],
                            gridx[x]             , gridy[y],
                            gridx[x+1]-gridx[x]+1, gridy[y+1]-gridy[y]+1
                            );
            a++;
        }
    }

    tokens = get_config_argv("listbox", "vscroll_gridx", &tokenCount);
    for (a=0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("listbox", "vscroll_gridy", &tokenCount);
    for (a=0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }
    a=0;
    for (y=0; y < 3; y++) {
        for (x=0; x < 3; x++) {
            gui_skin.listbox.vscroll.grid[a] = create_sub_bitmap(
                            gui__repository[GUI_BMP_OFS_LISTBOX+1],
                            gridx[x]             , gridy[y],
                            gridx[x+1]-gridx[x]+1, gridy[y+1]-gridy[y]+1
                            );
            a++;
        }
    }

}
void loadBarSkin() {
BITMAP *temp1 = load_bitmap("data/bar.bmp", NULL);
BITMAP *temp2 = load_bitmap("data/bar_filled.bmp", NULL);
gui_skin.bar.bg.grid[0] = create_bitmap(3,11);
gui_skin.bar.bg.grid[1] = create_bitmap(1,11);
gui_skin.bar.bg.grid[2] = create_bitmap(3,11);
blit(temp1, gui_skin.bar.bg.grid[0], 0, 0, 0, 0, 3, 11);
blit(temp1, gui_skin.bar.bg.grid[1], 4, 0, 0, 0, 1, 11);
blit(temp1, gui_skin.bar.bg.grid[2], 13, 0, 0, 0, 3, 11);
gui_skin.bar.bg.grid[3] = create_bitmap(3,11);
gui_skin.bar.bg.grid[4] = create_bitmap(1,11);
gui_skin.bar.bg.grid[5] = create_bitmap(3,11);
blit(temp2, gui_skin.bar.bg.grid[3], 0, 0, 0, 0, 3, 11);
blit(temp2, gui_skin.bar.bg.grid[4], 4, 0, 0, 0, 1, 11);
blit(temp2, gui_skin.bar.bg.grid[5], 13, 0, 0, 0, 3, 11);
}
void loadRadioSkin() {
gui_skin.radiobutton.normal = load_bitmap("data/Skin/radioout.bmp", NULL);
gui_skin.radiobutton.checked = load_bitmap("data/Skin/radioin.bmp", NULL);
gui_skin.radiobutton.disabled = load_bitmap("data/Skin/radioout.bmp", NULL);
gui_skin.radiobutton.disabled_checked = load_bitmap("data/Skin/radioin.bmp", NULL);
}
void loadPlusSkin() {
//BITMAP *temp1 = load_bitmap("data/bar.bmp", NULL);
//BITMAP *temp2 = load_bitmap("data/bar_filled.bmp", NULL);
gui_skin.plus.bg.grid[0] = load_bitmap("data/plus.bmp", NULL);
gui_skin.plus.bg.grid[1] = load_bitmap("data/plus_sel.bmp", NULL);
gui_skin.plus.bg.grid[2] = load_bitmap("data/plus_dis.bmp", NULL);
//blit(temp1, gui_skin.bar.bg.grid[0], 0, 0, 0, 0, 3, 11);
//blit(temp1, gui_skin.bar.bg.grid[1], 4, 0, 0, 0, 1, 11);
//blit(temp1, gui_skin.bar.bg.grid[2], 13, 0, 0, 0, 3, 11);
}
void loadDialogSkin() {
    char **tokens;
    int    tokenCount;
    int    gridx[4];
    int    gridy[4];
    int    a = 0;
    int    x,y;

    tokens = get_config_argv("dialog", "gridx", &tokenCount);
    for (a=0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("dialog", "gridy", &tokenCount);
    for (a=0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }

		gui__repository[GUI_BMP_OFS_DIALOG] = (BITMAP *)gui_gfx[5].dat;

    a=0;
    for (y=0; y < 3; y++) {
        for (x=0; x < 3; x++) {
            gui_skin.dialog.bg.grid[a] = create_sub_bitmap(
                            gui__repository[GUI_BMP_OFS_DIALOG],
                            gridx[x]             , gridy[y],
                            gridx[x+1]-gridx[x]+1, gridy[y+1]-gridy[y]+1
                            );
            a++;
        }
    }
}

void draw_skinned_rect(BITMAP*dst, LexSkinnedRect *skin, int x, int y,int w, int h) {
    BITMAP **grid = skin->grid;

    int w0 = grid[0]->w;
    int w1 = w - grid[0]->w -grid[2]->w;
    int w2 = grid[2]->w;
    int h0 = grid[0]->h;
    int h1 = h - grid[0]->h - grid[6]->h;
    int h2 = grid[6]->h;

    int cx,cy;

    cx = x; cy = y;
    masked_blit(grid[0], dst, 0, 0, cx, cy,grid[0]->w,grid[0]->h);
    cy += h0;
    masked_stretch_blit(grid[3], dst, 0, 0, grid[3]->w,grid[3]->h,cx, cy,w0,h1);
    cy += h1;
    masked_blit(grid[6], dst, 0, 0, cx, cy,grid[6]->w,grid[6]->h);

    cx += w0;
    cy  = y;
    masked_stretch_blit(grid[1], dst, 0, 0, grid[1]->w,grid[1]->h,cx, cy,w1,h0);
    cy += h0;
    masked_stretch_blit(grid[4], dst, 0, 0, grid[4]->w,grid[4]->h,cx, cy,w1,h1);
    cy += h1;
    masked_stretch_blit(grid[7], dst, 0, 0, grid[7]->w,grid[7]->h,cx, cy,w1,h2);

    cx += w1;
    cy  = y;
    masked_blit(grid[2], dst, 0, 0, cx, cy,grid[2]->w,grid[2]->h);
    cy += h0;
    masked_stretch_blit(grid[5], dst, 0, 0, grid[5]->w,grid[5]->h,cx, cy,w2,h1);
    cy += h1;
    masked_blit(grid[8], dst, 0, 0, cx, cy,grid[8]->w,grid[7]->h);
}


int gui_load_skin(const char* skinname) {
    gui__external_slider_callback = NULL;
    push_config_state();
    set_config_file(skinname);
    gui_gfx = load_datafile(get_config_string("skin", "gfx", 0));
    loadButtonSkin();
    loadSliderSkin();
    loadCheckboxSkin();
    loadRadioSkin();
    loadTextboxSkin();
    loadListboxSkin();
    loadDialogSkin();
    loadBarSkin();
    loadPlusSkin();
    pop_config_state();
    set_mouse_sprite((BITMAP *)gui_gfx[7].dat);

    return TRUE;
}

void gui_exit() {
    delete guiFont;
    delete guiTop;
    delete gui;

    delete guiInput;
    delete guiGraphics;
    delete imageLoader;

    gui_shutdown();
}

void gui_shutdown(void) {
    int a, b;

    /* Button */
    for (a=0; a < 3; a++) {
        for (b=0; b < 9 ; b++) {
            destroy_bitmap(gui_skin.button.background[a].grid[b]);
        }

    }
    /* Slider */
    for (a=0; a < 3; a++) {
        destroy_bitmap(gui_skin.slider.hSlider[a]);
        destroy_bitmap(gui_skin.slider.vSlider[a]);
    }
    destroy_bitmap(gui_skin.slider.hGrip);
    destroy_bitmap(gui_skin.slider.vGrip);

    /* Checkbox */
    destroy_bitmap(gui_skin.checkbox.normal);
    destroy_bitmap(gui_skin.checkbox.checked);
    destroy_bitmap(gui_skin.checkbox.disabled);
    destroy_bitmap(gui_skin.checkbox.disabled_checked);

    /* Radiobutton */
    destroy_bitmap(gui_skin.radiobutton.normal);
    destroy_bitmap(gui_skin.radiobutton.checked);
    destroy_bitmap(gui_skin.radiobutton.disabled);
    destroy_bitmap(gui_skin.radiobutton.disabled_checked);

    for (a=0; a < GUI_BMP_COUNT; a++) {
        destroy_bitmap(gui__repository[a]);
    }
}

/*
 * Draw text for gui widgets
 */
int gui_text(BITMAP *bmp, AL_CONST char *s, int x, int y, int color, int centre) {
    char tmp[1024];
    int hline_pos = -1;
    int len = 0;
    int in_pos = 0;
    int out_pos = 0;
    int pix_len, c;

    while (((c = ugetc(s+in_pos)) != 0) && (out_pos<(int)(sizeof(tmp)-ucwidth(0)))) {
        if (c == '&') {
            in_pos += uwidth(s+in_pos);
            c = ugetc(s+in_pos);
            if (c == '&') {
                out_pos += usetc(tmp+out_pos, '&');
                in_pos += uwidth(s+in_pos);
                len++;
            } else hline_pos = len;
        } else {
            out_pos += usetc(tmp+out_pos, c);
            in_pos += uwidth(s+in_pos);
            len++;
        }
    }
    usetc(tmp+out_pos, 0);
    pix_len = text_length(font, tmp);

    if (centre)x -= pix_len / 2;
    if (bmp) {
        textprintf_ex(bmp, font, x, y, color, -1, tmp);
        if (hline_pos >= 0) {
            c = ugetat(tmp, hline_pos);
            usetat(tmp, hline_pos, 0);
            hline_pos = text_length(font, tmp);
            c = usetc(tmp, c);
            usetc(tmp+c, 0);
            c = text_length(font, tmp);
            hline(bmp, x+hline_pos, y+text_height(font)-gui_font_baseline, x+hline_pos+c-1, color);
        }
    }
    return pix_len;
}

int tmw_text_proc(int msg, DIALOG *d, int c) {
    if (msg==MSG_DRAW) {
        gui_text(gui_bitmap, (char *)d->dp, d->x, d->y, d->fg, FALSE);
    }
    return D_O_K;
}


int tmw_button_proc(int msg, DIALOG *d, int c) {

    int rtm = 0;
    int col = 0;
    int ofs = 0;
    int ret = D_O_K;

    if (msg == MSG_DRAW) {
        rectfill(gui_bitmap, d->x, d->y, d->x + d->w, d->y+d->h, makecol(255,255,255));

        if (d->flags & D_DISABLED) {
            draw_skinned_rect(gui_bitmap, &gui_skin.button.background[3], d->x, d->y, d->w, d->h);
            col = gui_skin.button.textcolor[3];
        } else if (d->flags & D_SELECTED) {
            draw_skinned_rect(gui_bitmap, &gui_skin.button.background[2], d->x, d->y, d->w, d->h);
            col = gui_skin.button.textcolor[2];
            ofs = 1;
        } else if (d->flags & D_GOTMOUSE) {
            draw_skinned_rect(gui_bitmap, &gui_skin.button.background[1], d->x, d->y, d->w, d->h);
            col = gui_skin.button.textcolor[1];
        } else {
            draw_skinned_rect(gui_bitmap, &gui_skin.button.background[0], d->x, d->y, d->w, d->h);
            col = gui_skin.button.textcolor[0];
        }
      gui_text(gui_bitmap, (const char *)d->dp, d->x+d->w/2+ofs, d->y+d->h/2-text_height(font)/2+ofs, col, TRUE);
    ret = D_O_K;
  } else {
    /*if(msg==MSG_CLICK) {
      if(d->d1==1)((int)d->dp2) + 1;
      else if(d->d1==2)((int)d->dp2) - 1;
    }*/

    ret =  d_button_proc(msg,d,c);
    }
    return ret;
}

int tmw_slider_proc(int msg, DIALOG *d, int c) {
    int w   = 0;
    int h   = 0;
    int x,y;

    int ret = D_O_K;

  static int watchdog = 0;

  watchdog++;
  if (watchdog == 1) {
    gui__external_slider_callback = (int(__cdecl *)(void *, int))d->dp2;
    d->dp2 = (void*)reroute_slider_proc;
  }

    if (msg == MSG_DRAW) {
        if (d->w >= d->h) {
            //rectfill(gui_bitmap, d->x, d->y, d->x + d->w, d->y+d->h, d->bg);
            /* horiz */
            x = d->x;
            y = d->y + (d->h- gui_skin.slider.hSlider[0]->h)/2;
            masked_blit(gui_skin.slider.hSlider[0], gui_bitmap, 0, 0,  x, y, gui_skin.slider.hSlider[0]->w, gui_skin.slider.hSlider[0]->h);
            w   = d->w -gui_skin.slider.hSlider[0]->w - gui_skin.slider.hSlider[2]->w;
            x+= gui_skin.slider.hSlider[0]->w;

            masked_stretch_blit(
                    gui_skin.slider.hSlider[1], gui_bitmap,
                    0, 0,  gui_skin.slider.hSlider[1]->w, gui_skin.slider.hSlider[1]->h,
                    x, y, w, gui_skin.slider.hSlider[1]->h);

            x+=w;
            masked_blit(gui_skin.slider.hSlider[2], gui_bitmap, 0, 0,  x, y, gui_skin.slider.hSlider[2]->w, gui_skin.slider.hSlider[2]->h);

						if(d->d1==0)d->d1=1; // Fix by 0 division

            x  = d->x + ((d->w-gui_skin.slider.hGrip->w) * d->d2)/d->d1;
            y = d->y + (d->h - gui_skin.slider.hGrip->h)/2;
						if(!gui_bitmap)error("gui_bitmap");
						if(!gui_skin.slider.hGrip)error("hGrip");
            masked_blit(gui_skin.slider.hGrip, gui_bitmap, 0, 0,  x, y, gui_skin.slider.hGrip->w, gui_skin.slider.hGrip->h);
        } else {
            rectfill(gui_bitmap, d->x, d->y, d->x + d->w, d->y+d->h, d->bg);
            /* vertic */
            x = d->x+ (d->w- gui_skin.slider.vSlider[0]->w)/2;
            y = d->y;
            masked_blit(gui_skin.slider.vSlider[0], gui_bitmap, 0, 0,  x, y, gui_skin.slider.vSlider[0]->w, gui_skin.slider.vSlider[0]->h);
            h   = d->h - gui_skin.slider.vSlider[0]->h - gui_skin.slider.vSlider[2]->h;
            y  += gui_skin.slider.vSlider[0]->h;

            masked_stretch_blit(
                    gui_skin.slider.vSlider[1], gui_bitmap,
                    0, 0,  gui_skin.slider.vSlider[1]->w, gui_skin.slider.vSlider[1]->h,
                    x, y,  gui_skin.slider.vSlider[1]->w, h);

            y+=h;
            masked_blit(gui_skin.slider.vSlider[2], gui_bitmap, 0, 0,  x, y, gui_skin.slider.vSlider[2]->w, gui_skin.slider.vSlider[2]->h);

            y = d->y + d->h - (((d->h-gui_skin.slider.vGrip->h) * d->d2)/d->d1)-gui_skin.slider.vGrip->h;
            x = d->x + (d->w - gui_skin.slider.vGrip->w)/2;
            if (gui_skin.slider.vGrip->w % 2 !=0) {
                x++;
            }
            masked_blit(gui_skin.slider.vGrip, gui_bitmap, 0, 0,  x, y, gui_skin.slider.vGrip->w, gui_skin.slider.vGrip->h);
        }
        //textprintf(gui_bitmap, font,10, 10, makecol(255,255,255), "%i", d->d2);
    } else {
      if(d->d1==0)d->d1 = 1;
      ret = d_slider_proc(msg,d,c);
    }

  if (watchdog == 1) {
    d->dp2 = (void*)gui__external_slider_callback;
  }
  watchdog--;
    return ret;
}


int tmw_radio_proc(int msg, DIALOG *d, int c) {
    BITMAP *box = NULL;
    int     x, y;
    int     tx, ty, l;
    int     rtm = 0;
    int     col = 0;


    if (msg == MSG_DRAW) {
        rectfill(gui_bitmap, d->x, d->y, d->x + d->w, d->y+d->h, d->bg);
        if (d->flags & D_SELECTED) {
            if (d->flags & D_DISABLED) {
                box = gui_skin.radiobutton.disabled_checked;
            } else {
                box = gui_skin.radiobutton.checked;
            }
        } else if (d->flags & D_DISABLED) {
            box = gui_skin.radiobutton.disabled;
        } else {
            box = gui_skin.radiobutton.normal;
        }

        if (d->flags & D_DISABLED) {
            col = gui_skin.radiobutton.textcolor[1];
        } else {
            col = gui_skin.radiobutton.textcolor[0];
        }

        if (d->dp != NULL) {
            l = gui_strlen((const char *)d->dp);
        } else {
            l = 0;
        }

        if (d->d2 != 0) {
            x  = d->x;
            tx = x + box->w + box->w/2;
        } else {
            x  = d->x + d->w - box->w;
            tx = x - box->w/2 - l;
        }
        y  = d->y + (d->h - box->h)/ 2;
        ty = d->y + (d->h - text_height(font)) / 2;

        masked_blit(box, gui_bitmap, 0, 0, x, y, box->w, box->h);
        if (d->dp != NULL) {
            gui_text(gui_bitmap, (const char *)d->dp, tx, ty, col, 0);
        }


    } else {
        return d_radio_proc(msg, d, c);
    }
    return D_O_K;
}


int tmw_list_proc(int msg, DIALOG *d, int c) {
  static int ignoreRedraw = FALSE;
  int itemCount     = 0;
  int firstItem     = d->d2;
  int lastItem      = 0;
  int selectedItem  = d->d1;
  int x,y,delta;
  int a, col;
  int w, h          = 0;
  int rtm           = 0;
  int cl, cr, cb, ct;
  int th            = text_height(font);

  int vscroll = 0;
  int sliderh = 10;
  int slidery = 0;

  (*(getfuncptr)d->dp)(-1, &itemCount);
  w = d->w - gui_skin.listbox.bg.grid[0]->w - gui_skin.listbox.bg.grid[2]->w;
	h = d->h - gui_skin.listbox.bg.grid[1]->h - gui_skin.listbox.bg.grid[7]->h;
	lastItem = MIN(itemCount-1, firstItem + h / text_height(font));

  if (msg == MSG_DRAW) {
    if (ignoreRedraw) {
      return D_O_K;
    }
    rectfill(gui_bitmap, d->x, d->y, d->x + d->w, d->y+d->h, d->bg);
		draw_skinned_rect(gui_bitmap, &gui_skin.listbox.bg, d->x, d->y, d->w, d->h);
    (*(getfuncptr)d->dp)(-1, &itemCount);
    vscroll = (h/th) < (itemCount-1);
    if (vscroll) {
      w = d->w - 17 - gui_skin.listbox.bg.grid[0]->w;
      draw_skinned_rect(gui_bitmap, &gui_skin.listbox.bg, d->x+d->w-15, d->y+1, 14, d->h-2);
      sliderh = MAX(((d->h-2)* (h / th)) / itemCount, gui_skin.listbox.bg.grid[0]->h*2);
      slidery = ((d->h-2-sliderh) * firstItem) / (itemCount);
      slidery+= d->y+1;
      draw_skinned_rect(gui_bitmap, &gui_skin.listbox.vscroll, d->x+d->w-13, slidery, 11, sliderh);
    }

    if (gui_bitmap->clip) {
			cl = gui_bitmap->cl;
			ct = gui_bitmap->ct;
			cr = gui_bitmap->cr;
			cb = gui_bitmap->cb;
		} else {
			cl=ct=0;
			cr=gui_bitmap->w;
			cb=gui_bitmap->h;
		}
		x = d->x + gui_skin.listbox.bg.grid[0]->w;
		y = d->y + gui_skin.listbox.bg.grid[0]->h;
		set_clip_rect(gui_bitmap, x,y, x+w, y+h);
    if (d->flags & D_DISABLED) {
			col = gui_skin.listbox.textcolor[3];
			for (a=firstItem; a < lastItem; a++) {
				textprintf_ex(gui_bitmap, font, x, y, col, -1, (*(getfuncptr)d->dp)(a, 0));
				y += text_height(font);
			}
		} else {
			for (a=firstItem; a <= lastItem; a++) {
				if (a==d->d1) {
					col = gui_skin.listbox.textcolor[1];
					rectfill(gui_bitmap, x, y, x+w, y+text_height(font)-1, gui_skin.listbox.textcolor[2]);
				} else {
					col = gui_skin.listbox.textcolor[0];
				}
				textprintf_ex(gui_bitmap, font, x, y, col, -1, (*(getfuncptr)d->dp)(a, 0));
				y += text_height(font);
			}
		}
    set_clip_rect(gui_bitmap, cl, ct, cr, cb);
	} else if (msg == MSG_CLICK) {
		x = d->x + gui_skin.listbox.bg.grid[0]->w;
		y = d->y + gui_skin.listbox.bg.grid[0]->h;
		sliderh = MAX(((d->h-2)* (h / th)) / itemCount, gui_skin.listbox.bg.grid[0]->h*2);
		//sliderh = ((d->h-2)* (h / th)) / itemCount;
		slidery = ((d->h-2-sliderh) * firstItem) / (itemCount);
		slidery+= d->y+1;
		if (mouse_x > (d->x + d->w - 14) && mouse_x < (d->x+d->w-1)) {
			// Ok, scroll bar
			if (mouse_y >= slidery && mouse_y < slidery + sliderh) {
				delta= mouse_y - slidery;
				while (mouse_b) {
					a  = mouse_y - delta - d->y -1;
					a *= itemCount;
					a /= (d->h-2);
					a  = MID(0, a, itemCount- h/th);
          if (a != d->d2) {
						d->d2 = a;
						scare_mouse();
						object_message(d, MSG_DRAW, 0);
						unscare_mouse();
					}
					slidery = ((d->h-2) * firstItem) / (itemCount);
					slidery+= d->y+1;
				}
			} else if (mouse_y < slidery) {
				a = d->d2 - (h/th)+1;
				a = MID(0, a, itemCount- h/th);

        d->d2 = a;
        scare_mouse();
				object_message(d, MSG_DRAW, 0);
				unscare_mouse();
        while (mouse_b) {
				}
			} else if (mouse_y > slidery + sliderh) {
				a = d->d2 + (h/th)-1;
				a = MID(0, a, itemCount- h/th);
				d->d2 = a;
        scare_mouse();
				object_message(d, MSG_DRAW, 0);
				unscare_mouse();
        while (mouse_b) {
				}
			}
		} else if (mouse_x >= x && mouse_x < x+w && mouse_y >= y && mouse_y < y+h) {
      while (mouse_b) {
				a = firstItem + (mouse_y-y) / text_height(font);
        if (a <= lastItem && a != selectedItem) {
					d->d1 = selectedItem = a;
					scare_mouse();
					object_message(d, MSG_DRAW, 0);
					unscare_mouse();
				}
			}
		}
	} else {
		ignoreRedraw = (msg == MSG_GOTFOCUS || msg == MSG_LOSTFOCUS);
		a =  d_list_proc(msg, d, c);
    if (a == D_USED_CHAR) {
			if (d->d1 < d->d2) {
				if (d->d1 > 0) {
					d->d1 = d->d2;
				}
			} else if (d->d1 > d->d2 + h/th -1) {
				d->d2 = d->d1 - h/th + 1;
			}
		}
    return a;
	}
	return D_O_K;
}
int tmw_plus_proc(int msg, DIALOG *d, int c)
{
//d->d1 = 0;
bool draw = false;

if(mouse_b & 1)
	{
	if(d->x+d->w > mouse_x && d->x < mouse_x && d->y+d->h > mouse_y && d->y < mouse_y)
		if(d->d2==1)
			{
				//d->d1 = 1;
				(*(int(__cdecl *)(void *, int))d->dp)(NULL, d->d1);
				masked_blit(gui_skin.plus.bg.grid[1], gui_bitmap, 0, 0, d->x, d->y, gui_bitmap->w, gui_bitmap->h);
				draw = true;
			} else {
				masked_blit(gui_skin.plus.bg.grid[0], gui_bitmap, 0, 0, d->x, d->y, gui_bitmap->w, gui_bitmap->h);
				draw = true;
			}
	} else {
		if (!d->d2) {
 			//disable
			masked_blit(gui_skin.plus.bg.grid[2], gui_bitmap, 0, 0, d->x, d->y, gui_bitmap->w, gui_bitmap->h);
			draw = true;
		}
	}
		if(!draw)
			masked_blit(gui_skin.plus.bg.grid[0], gui_bitmap, 0, 0, d->x, d->y, gui_bitmap->w, gui_bitmap->h);


return D_O_K;
}

/* Dialog box with left centered head */
int tmw_dialog_proc(int msg, DIALOG *d, int c) {
  int rtm;
  int x, y;

	switch(msg) {

		case MSG_CLICK:
      if(mouse_y<d->y+gui_skin.dialog.bg.grid[1]->h) {
        d->d1 = mouse_x - d->x;
        d->d2 = mouse_y - d->y;
      }
			break;
		case MSG_DRAW:
      if((mouse_b & 1)&&(d->d1>=0)&&(d->d2>=0)) {
        x = mouse_x-d->d1;
        y = mouse_y-d->d2;
        if(x<15) {
          x=0;
          position_mouse(d->d1, mouse_y);
        }
        if(y<15) {
          y=0;
          position_mouse(mouse_x, d->d2);
        }
        if(x+d->w>=SCREEN_W-15) {
          x=SCREEN_W-d->w;
          position_mouse(x+d->d1, mouse_y);
        }
        if(y+d->h>=SCREEN_H-15) {
          y=SCREEN_H-d->h;
          position_mouse(mouse_x, y+d->d2);
        }
        position_dialog(d, x, y);
      } else {
        d->d1 = -1;
        d->d2 = -1;
      }
      draw_skinned_rect(gui_bitmap, &gui_skin.dialog.bg, d->x, d->y, d->w, d->h);

      textprintf_centre_ex(gui_bitmap, font,
				d->x + d->w/2,
				d->y + (gui_skin.dialog.bg.grid[1]->h - text_height(font))/2, d->fg, -1, "%s", d->dp);

			break;
	}
  return D_O_K;
}


/**
	dialog box w/ left aligned head
*/
int tmw_ldialog_proc(int msg, DIALOG *d, int c) {
    int rtm;
  int x, y;

  if (msg == MSG_CLICK) {
    if(mouse_y < d->y + gui_skin.dialog.bg.grid[1]->h) {
            //drag = true;
            d->d1 = mouse_x - d->x;
            d->d2 = mouse_y - d->y;
    }
    } else if (msg == MSG_DRAW) {
    if((mouse_b & 1)&&(d->d1>=0)&&(d->d2>=0)) {//(drag==true)) {
      x = mouse_x-d->d1;
      y = mouse_y-d->d2;
      if(x<15) {
        x=0;
        position_mouse(d->d1, mouse_y);
      }
      if(y<15) {
        y=0;
        position_mouse(mouse_x, d->d2);
      }
      if(x+d->w>=785) {
        x=800-d->w;
        position_mouse(x+d->d1, mouse_y);
      }
      if(y+d->h>=585) {
        y=600-d->h;
        position_mouse(mouse_x, y+d->d2);
      }
          position_dialog(active_dialog, x, y);
    } else {
      //drag = false;
      d->d1 = -1;
      d->d2 = -1;
    }
        draw_skinned_rect(gui_bitmap, &gui_skin.dialog.bg, d->x, d->y, d->w, d->h);

        textprintf_ex(gui_bitmap, font, d->x + 4, d->y + (gui_skin.dialog.bg.grid[1]->h - text_height(font))/2, d->fg, -1, "%s", d->dp);
    }
    return D_O_K;
}

int reroute_slider_proc(void *dp3, int d2) {
    int ret = 0;

    if (gui__external_slider_callback != NULL) {
        ret = gui__external_slider_callback(dp3, d2);
    }
    return ret;
}

// Helper function to draw a scrollable bar
void _gui_draw_scrollable_frame(DIALOG *d, int listsize, int offset, int height, int fg_color, int bg) {
    int i, len;
    int xx, yy;

    /* create and draw the scrollbar */
    i = ((d->h-5) * height + listsize/2) / listsize;
    xx = d->x+d->w-10;
    yy = d->y;

  if (offset > 0) {
        len = (((d->h-5) * offset) + listsize/2) / listsize;
    } else len = 0;
    if (yy+i < d->y+d->h-3) {
    draw_skinned_rect(gui_bitmap, &gui_skin.listbox.vscroll, xx, yy+len, 10, i);
  } else {
        draw_skinned_rect(gui_bitmap, &gui_skin.listbox.vscroll, xx, yy, 10, d->h-3);
  }
}

/* _gui_draw_textbox:
 *  Helper function to draw a textbox object.
 */
void _gui_draw_textbox(char *thetext, int *listsize, int draw, int offset,
       int wword, int tabsize, int x, int y, int w, int h,
       int disabled, int fore, int deselect, int disable)
{
   int fg = fore;
   int y1 = y+4;
   int x1;
   int len;
   int ww = w-6;
   char s[16];
   char text[16];
   char space[16];
   char *printed = text;
   char *scanned = text;
   char *oldscan = text;
   char *ignore = NULL;
   char *tmp, *ptmp;
   int width;
   int line = 0;
   int i = 0;
   int noignore;
   int rtm;

   usetc(s+usetc(s, '.'), 0);
   usetc(text+usetc(text, ' '), 0);
   usetc(space+usetc(space, ' '), 0);

   /* find the correct text */
   if (thetext != NULL) {
      printed = thetext;
      scanned = thetext;
   }

   /* choose the text color */
   if (disabled)
      fg = disable;

   /* loop over the entire string */
   while (1) {
      width = 0;

      /* find the next break */
      while (ugetc(scanned)) {
   /* check for a forced break */
   if (ugetc(scanned) == '\n') {
      scanned += uwidth(scanned);

      /* we are done parsing the line end */
      break;
   }

   /* the next character length */
   usetc(s+usetc(s, ugetc(scanned)), 0);
   len = text_length(font, s);

   /* modify length if its a tab */
   if (ugetc(s) == '\t')
      len = tabsize * text_length(font, space);

   /* check for the end of a line by excess width of next char */
   if (width+len >= ww) {
      /* we have reached end of line do we go back to find start */
      if (wword) {
         /* remember where we were */
         oldscan = scanned;
         noignore = FALSE;

         /* go backwards looking for start of word */
         while (!uisspace(ugetc(scanned))) {
      /* don't wrap too far */
      if (scanned == printed) {
         /* the whole line is filled, so stop here */
         tmp = ptmp = scanned;
         while (ptmp != oldscan) {
      ptmp = tmp;
      tmp += uwidth(tmp);
         }
         scanned = ptmp;
         noignore = TRUE;
         break;
      }
      /* look further backwards to wrap */
      tmp = ptmp = printed;
      while (tmp < scanned) {
         ptmp = tmp;
         tmp += uwidth(tmp);
      }
      scanned = ptmp;
         }
         /* put the space at the end of the line */
         if (!noignore) {
      ignore = scanned;
      scanned += uwidth(scanned);
         }
         else
      ignore = NULL;

         /* check for endline at the convenient place */
         if (ugetc(scanned) == '\n')
      scanned += uwidth(scanned);
      }
      /* we are done parsing the line end */
      break;
   }

   /* the character can be added */
   scanned += uwidth(scanned);
   width += len;
      }

      /* check if we are to print it */
      if ((draw) && (line >= offset) && (y1+text_height(font) < (y+h-3))) {
   x1 = x+4;

   /* the initial blank bit */
   //rectfill(gui_bitmap, x+2, y1, x1-1, y1+text_height(font), deselect);

   /* print up to the marked character */
   while (printed != scanned) {
      /* do special stuff for each charater */
      switch (ugetc(printed)) {

         case '\r':
         case '\n':
      /* don't print endlines in the text */
      break;

         /* possibly expand the tabs */
         case '\t':
      for (i=0; i<tabsize; i++) {
         usetc(s+usetc(s, ' '), 0);
         textprintf_ex(gui_bitmap, font, x1, y1, fg, -1, s);
         x1 += text_length(font, s);
      }
      break;

         /* print a normal character */
         default:
      if (printed != ignore) {
         usetc(s+usetc(s, ugetc(printed)), 0);
         textprintf_ex(gui_bitmap, font, x1, y1, fg, -1, s);
         x1 += text_length(font, s);
      }
      }

      /* goto the next character */
      printed += uwidth(printed);
   }
   /* the last blank bit */
   /*if (x1 <= x+w-3)
      rectfill(gui_bitmap, x1, y1, x+w-3, y1+alfont_text_height(font)-1, deselect);*/

   /* print the line end */
   y1 += text_height(font);
      }
      printed = scanned;

      /* we have done a line */
      line++;

      /* check if we are at the end of the string */
      if (!ugetc(printed)) {
   /* the under blank bit */
   /*if (draw)
      rectfill(gui_bitmap, x+1, y1, x+w-3, y+h-1, deselect);*/

   /* tell how many lines we found */
   *listsize = line;
   return;
      }
   }
}

int tmw_textbox_proc(int msg, DIALOG *d, int c) {
   int height, bar, ret = D_O_K;
   int start, top, bottom, l;
   int used, delta;
   int fg_color = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;

   /* calculate the actual height */
   height = (d->h-8) / text_height(font);

   switch (msg) {

      case MSG_START:
   /* measure how many lines of text we contain */
   _gui_draw_textbox((char *)d->dp, &d->d1,
           0, /* DONT DRAW anything */
           d->d2, !(d->flags & D_SELECTED), 8,
           d->x, d->y, d->w, d->h,
           (d->flags & D_DISABLED),
           0, 0, 0);
   break;

      case MSG_DRAW:
   /* tell the object to sort of draw, but only calculate the listsize */
   _gui_draw_textbox((char *)d->dp, &d->d1,
           0, /* DONT DRAW anything */
           d->d2, !(d->flags & D_SELECTED), 8,
           d->x, d->y, d->w, d->h,
           (d->flags & D_DISABLED),
           0, 0, 0);

   if (d->d1 > height) {
      bar = 12;
   }
   else {
      bar = 0;
      d->d2 = 0;
   }

   /* now do the actual drawing */
   _gui_draw_textbox((char *)d->dp, &d->d1, 1, d->d2,
           !(d->flags & D_SELECTED), 8,
           d->x, d->y, d->w-bar, d->h,
           (d->flags & D_DISABLED),
           fg_color, d->bg, gui_mg_color);

   /* draw the frame around */
   _gui_draw_scrollable_frame(d, d->d1, d->d2, height, fg_color, d->bg);

   break;

      case MSG_CLICK:
         /* figure out if it's on the text or the scrollbar */
   bar = (d->d1 > height);

   if ((!bar) || (gui_mouse_x() < d->x+d->w-13)) /* clicked on the text area */
     ret = D_O_K;
   else /* clicked on the scroll area */
     //_handle_scrollable_scroll_click(d, d->d1, &d->d2, height);
   break;

      case MSG_CHAR:
   start = d->d2;
   used = D_USED_CHAR;

   if (d->d1 > 0) {
      if (d->d2 > 0)
         top = d->d2+1;
      else
         top = 0;

      l = (d->h-8)/text_height(font);

      bottom = d->d2 + l - 1;
      if (bottom >= d->d1-1)
         bottom = d->d1-1;
      else
         bottom--;

      if ((c>>8) == KEY_UP)
         d->d2--;
      else if ((c>>8) == KEY_DOWN)
         d->d2++;
      else if ((c>>8) == KEY_HOME)
         d->d2 = 0;
      else if ((c>>8) == KEY_END)
         d->d2 = d->d1-l;
      else if ((c>>8) == KEY_PGUP)
         d->d2 -= (bottom-top) ? bottom-top : 1;
      else if ((c>>8) == KEY_PGDN)
         d->d2 += (bottom-top) ? bottom-top : 1;
      else
         used = D_O_K;

      /* make sure that the list stays in bounds */
      if (d->d2 > d->d1-l)
         d->d2 = d->d1-l;
      if (d->d2 < 0)
         d->d2 = 0;
   }
   else
      used = D_O_K;

   /* if we changed something, better redraw... */
   if (d->d2 != start)
      d->flags |= D_DIRTY;

   ret = used;
   break;

      case MSG_WHEEL:
   l = (d->h-8)/text_height(font);
   delta = (l > 3) ? 3 : 1;

   /* scroll, making sure that the list stays in bounds */
   start = d->d2;
   d->d2 = (c > 0) ? MAX(0, d->d2-delta) : MIN(d->d1-l, d->d2+delta);

   /* if we changed something, better redraw... */
   if (d->d2 != start)
      d->flags |= D_DIRTY;

   ret = D_O_K;
   break;

      case MSG_WANTFOCUS:
   /* if we don't have a scrollbar we can't do anything with the focus */
   if (d->d1 > height)
      ret = D_WANTFOCUS;
   break;

      default:
   ret = D_O_K;
   }

   return ret;
}

int tmw_bitmap_proc(int msg, DIALOG *d, int c) {
  if(msg==MSG_DRAW) {
    draw_skinned_rect(gui_bitmap, &gui_skin.textbox.bg, d->x, d->y, d->w, d->h);
    if(d->dp!=NULL)
            masked_blit(((BITMAP *)d->dp), gui_bitmap, 0, 0, d->x+(d->w-d->d1)/2, d->y+2, d->d1, d->d2);
  }
  return D_O_K;
}

void ok(const char *title, const char *message) {
DIALOG alert_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,     0,    0,  0,     60,    0,  -1,    0,    0,          0,                      0,    (void *)title,    NULL, NULL  },
   { tmw_text_proc,       2,   22,  0,      0,    0,   0,    0,    0,          0,                      0,    (void *)message,  NULL, NULL  },
   { tmw_button_proc,     0,   40,  44,    18,    0,  -1,    'o',  D_EXIT,    -1,                      0,    (char *)"&Ok",    NULL, NULL  },
   { NULL,                0,    0,   0,     0,    0,   0,    0,    0,          0,                      0,    NULL,             NULL, NULL  }};

   BITMAP *temp = gui_bitmap;
   gui_bitmap = screen;
   show_mouse(screen);
   alert_dialog[0].w = text_length(font, message)+4;
   alert_dialog[1].w = text_length(font, message);
   alert_dialog[1].h = text_height(font);
   alert_dialog[2].x = text_length(font, message)/2-22;
   position_dialog(alert_dialog, 400-alert_dialog[0].w/2, 270);
   do_dialog(alert_dialog, 2);
   show_mouse(NULL);
   gui_bitmap = temp;
}

unsigned int yes_no(const char *title, const char *message) {
  unsigned int ret;
  DIALOG alert_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,     0,    0,  0,     60,    0,  -1,    0,    0,          0,                      0,    (void *)title,    NULL, NULL  },
   { tmw_text_proc,       2,   22,  0,      0,    0,   0,    0,    0,          0,                      0,    (void *)message,  NULL, NULL  },
   { tmw_button_proc,     0,   40,  44,    18,    0,  -1,    'o',  D_EXIT,    -1,                      0,    (char *)"&Yes",   NULL, NULL  },
   { tmw_button_proc,     0,   40,  44,    18,    0,  -1,    'o',  D_EXIT,    -1,                      0,    (char *)"&No",    NULL, NULL  },
   { NULL,                0,    0,   0,     0,    0,   0,    0,    0,          0,                      0,    NULL,             NULL, NULL  }};

   BITMAP *temp = gui_bitmap;
   gui_bitmap = screen;
   show_mouse(screen);
   int width = text_length(font, message)+4;
   if(width<100)width=100;
   alert_dialog[0].w = width;
   alert_dialog[1].w = text_length(font, message);
   alert_dialog[1].h = text_height(font);
   alert_dialog[2].x = width/2-46;
   alert_dialog[2].x = width/2+2;
   position_dialog(alert_dialog, 400-width/2, 270);
   ret = do_dialog(alert_dialog, 3);
   show_mouse(NULL);
   gui_bitmap = temp;
   return ret-2;
}


