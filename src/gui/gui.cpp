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

// The currently active skin
LexSkin gui_skin;
BITMAP *gui_bitmap;
DATAFILE *gui_gfx;


// Guichan stuff
Gui* gui;
gcn::AllegroGraphics* guiGraphics;     // Graphics driver
gcn::Container* guiTop;                // The top container


Gui::Gui(BITMAP *screen)
{
    // Set graphics
    guiGraphics = new gcn::AllegroGraphics();
    guiGraphics->setTarget(screen);

    // Set input
    guiInput = new AllegroInput();

    // Set image loader
    imageLoader = new gcn::AllegroImageLoader();
    gcn::Image::setImageLoader(imageLoader);

    // Initialize top GUI widget
    guiTop = new gcn::Container();
    guiTop->setDimension(gcn::Rectangle(0, 0, SCREEN_W, SCREEN_H));
    guiTop->setOpaque(false);

    // Create focus handler
    focusHandler = new gcn::FocusHandler();
    guiTop->_setFocusHandler(focusHandler);

    // Set global font
    guiFont = new gcn::ImageFont("./data/graphic/fixedfont.bmp",
            " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:@"
            "!\"$%&/=?^+*#[]{}()<>_;'.,\\|-~`"
            );
    gcn::Widget::setGlobalFont(guiFont);
}

Gui::~Gui()
{
    delete guiFont;
    delete guiTop;
    delete imageLoader;
    delete guiInput;
    delete guiGraphics;
    delete focusHandler;
}

void Gui::update()
{
    logic();
    draw();

    // Draw the mouse
    draw_sprite(gui_bitmap, mouse_sprite, mouse_x, mouse_y);
}

void Gui::logic()
{
    guiInput->_pollInput();

    while (!guiInput->isMouseQueueEmpty())
    {
        gcn::MouseInput mi = guiInput->dequeueMouseInput();
        gcn::Widget* focused = focusHandler->getFocused();

        if (mi.x > 0 && mi.y > 0 &&
                guiTop->getDimension().isPointInRect(mi.x, mi.y))
        {
            if (!topHasMouse) {
                guiTop->_mouseInMessage();
                topHasMouse = true;
            }

            gcn::MouseInput mio = mi;
            mio.x -= guiTop->getX();
            mio.y -= guiTop->getY();

            if (!guiTop->hasFocus()) {
                guiTop->_mouseInputMessage(mio);
            }
        }
        else {
            if (topHasMouse) {
                guiTop->_mouseOutMessage();
                topHasMouse = false;
            }
        }

        if (focusHandler->getFocused() && focused == focusHandler->getFocused())
        {
            int xOffset, yOffset;
            focused->getAbsolutePosition(xOffset, yOffset);

            gcn::MouseInput mio = mi;
            mio.x -= xOffset;
            mio.y -= yOffset;
            focused->_mouseInputMessage(mio);
        }
    }

    while (!guiInput->isKeyQueueEmpty())
    {
        gcn::KeyInput ki = guiInput->dequeueKeyInput();

        // Handle tabbing
        if (ki.getKey().getValue() == gcn::Key::TAB &&
                ki.getType() == gcn::KeyInput::PRESS)
        {
            if (ki.getKey().isShiftPressed()) {
                focusHandler->tabPrevious();
            }
            else {
                focusHandler->tabNext();
            }
        }
        else {
            // Send key inputs to the focused widgets
            gcn::Widget* focused = focusHandler->getFocused();
            if (focused)
            {
                if (focused->isFocusable()) {
                    focused->_keyInputMessage(ki);
                }
                else {
                    focusHandler->focusNone();
                }
            }
        }
    }

    guiTop->logic();
}

void Gui::draw()
{
    guiGraphics->_beginDraw();

    guiGraphics->pushClipArea(guiTop->getDimension());
    guiTop->draw(guiGraphics);
    guiGraphics->popClipArea();

    guiGraphics->_endDraw();
}

void Gui::focusNone()
{
    focusHandler->focusNone();
}


void init_gui(BITMAP *bitmap, const char *skin) {
    gui = new Gui(bitmap);

    gui_bitmap = bitmap;
    gui_load_skin(skin);
    show_mouse(NULL);
}


void loadButtonSkin() {
    char **tokens;
    int tokenCount;
    int gridx[4];
    int gridy[4];
    int a = 0;
    int x, y, mode;

    tokens = get_config_argv("button", "gridx", &tokenCount);
    for (a = 0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("button", "gridy", &tokenCount);
    for (a = 0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }

    tokens = get_config_argv("button", "textcol_norm", &tokenCount);
    gui_skin.button.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens = get_config_argv("button", "textcol_hilite", &tokenCount);
    gui_skin.button.textcolor[1] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens = get_config_argv("button", "textcol_pressed", &tokenCount);
    gui_skin.button.textcolor[2] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens = get_config_argv("button", "textcol_disabled", &tokenCount);
    gui_skin.button.textcolor[3] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));

    gui__repository[GUI_BMP_OFS_BUTTON + 0] = (BITMAP *)gui_gfx[0].dat;
    gui__repository[GUI_BMP_OFS_BUTTON + 1] = (BITMAP *)gui_gfx[2].dat;
    gui__repository[GUI_BMP_OFS_BUTTON + 2] = (BITMAP *)gui_gfx[3].dat;
    gui__repository[GUI_BMP_OFS_BUTTON + 3] = (BITMAP *)gui_gfx[1].dat;

    for (mode = 0; mode < 4; mode++) {
        a = 0;
        for (y = 0; y < 3; y++) {
            for (x = 0; x < 3; x++) {
                gui_skin.button.background[mode].grid[a] = create_sub_bitmap(
                        gui__repository[GUI_BMP_OFS_BUTTON + mode],
                        gridx[x], gridy[y],
                        gridx[x + 1] - gridx[x] + 1, gridy[y + 1] - gridy[y]+1
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
    if (!gui__repository[GUI_BMP_OFS_SLIDER])alert("","","","","",0,0);

    tokens = get_config_argv("slider", "slider_h", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);

    tokens = get_config_argv("slider", "slider_h_ofs", &tokenCount);
    o1 = atoi(tokens[0]); o2 = atoi(tokens[1]);

    gui_skin.slider.hSlider[0] = create_sub_bitmap(gui__repository[GUI_BMP_OFS_SLIDER], x , y, o1-x, h);
    gui_skin.slider.hSlider[1] = create_sub_bitmap(gui__repository[GUI_BMP_OFS_SLIDER], o1, y, o2-o1, h);
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
    gui_skin.slider.vGrip = create_sub_bitmap(gui__repository[GUI_BMP_OFS_SLIDER], x, y, w, h);

    tokens = get_config_argv("slider", "handle_h", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    gui_skin.slider.hGrip = create_sub_bitmap(gui__repository[GUI_BMP_OFS_SLIDER], x, y, w, h);
}

void loadCheckboxSkin()  {
    int x, y, w,h;
    char **tokens;
    int tokenCount;

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

    tokens = get_config_argv("button", "textcol_norm", &tokenCount);
    gui_skin.checkbox.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens = get_config_argv("button", "textcol_disabled", &tokenCount);
    gui_skin.checkbox.textcolor[1] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
}

void loadTextboxSkin() {
    char **tokens;
    int tokenCount;
    int gridx[4];
    int gridy[4];
    int a = 0;
    int x, y;

    tokens = get_config_argv("textbox", "gridx", &tokenCount);
    for (a = 0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("textbox", "gridy", &tokenCount);
    for (a = 0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }

    tokens = get_config_argv("textbox", "textcol_norm", &tokenCount);
    gui_skin.textbox.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens = get_config_argv("textbox", "textcol_disabled", &tokenCount);
    gui_skin.textbox.textcolor[1] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));

    gui__repository[GUI_BMP_OFS_TEXTBOX] = (BITMAP *)gui_gfx[9].dat;

    a = 0;
    for (y = 0; y < 3; y++) {
        for (x = 0; x < 3; x++) {
            gui_skin.textbox.bg.grid[a] = create_sub_bitmap(
                    gui__repository[GUI_BMP_OFS_TEXTBOX],
                    gridx[x], gridy[y],
                    gridx[x + 1] - gridx[x] + 1, gridy[y + 1] - gridy[y] + 1
                    );
            a++;
        }
    }
}

void loadListboxSkin() {
    char **tokens;
    int tokenCount;
    int gridx[4];
    int gridy[4];
    int a = 0;
    int x, y;

    tokens = get_config_argv("listbox", "gridx", &tokenCount);
    for (a = 0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("listbox", "gridy", &tokenCount);
    for (a = 0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }

    tokens = get_config_argv("listbox", "textcol_norm", &tokenCount);
    gui_skin.listbox.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens = get_config_argv("listbox", "textcol_selected", &tokenCount);
    gui_skin.listbox.textcolor[1] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens = get_config_argv("listbox", "textbg_selected", &tokenCount);
    gui_skin.listbox.textcolor[2] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens = get_config_argv("listbox", "textcol_disabled", &tokenCount);
    gui_skin.listbox.textcolor[3] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));

    gui__repository[GUI_BMP_OFS_LISTBOX + 0] = (BITMAP*)gui_gfx[6].dat;
    gui__repository[GUI_BMP_OFS_LISTBOX + 1] = (BITMAP*)gui_gfx[10].dat;

    a = 0;
    for (y = 0; y < 3; y++) {
        for (x = 0; x < 3; x++) {
            gui_skin.listbox.bg.grid[a] = create_sub_bitmap(
                    gui__repository[GUI_BMP_OFS_LISTBOX],
                    gridx[x], gridy[y],
                    gridx[x + 1] - gridx[x] + 1, gridy[y + 1] - gridy[y] + 1
                    );
            a++;
        }
    }

    tokens = get_config_argv("listbox", "vscroll_gridx", &tokenCount);
    for (a = 0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("listbox", "vscroll_gridy", &tokenCount);
    for (a = 0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }
    a = 0;
    for (y = 0; y < 3; y++) {
        for (x = 0; x < 3; x++) {
            gui_skin.listbox.vscroll.grid[a] = create_sub_bitmap(
                    gui__repository[GUI_BMP_OFS_LISTBOX+1],
                    gridx[x], gridy[y],
                    gridx[x + 1] - gridx[x] + 1, gridy[y + 1] - gridy[y] + 1
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
    int tokenCount;
    int gridx[4];
    int gridy[4];
    int a = 0;
    int x, y;

    tokens = get_config_argv("dialog", "gridx", &tokenCount);
    for (a = 0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("dialog", "gridy", &tokenCount);
    for (a = 0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }

    gui__repository[GUI_BMP_OFS_DIALOG] = (BITMAP *)gui_gfx[5].dat;

    a = 0;
    for (y = 0; y < 3; y++) {
        for (x = 0; x < 3; x++) {
            gui_skin.dialog.bg.grid[a] = create_sub_bitmap(
                            gui__repository[GUI_BMP_OFS_DIALOG],
                            gridx[x]             , gridy[y],
                            gridx[x+1]-gridx[x]+1, gridy[y+1]-gridy[y]+1
                            );
            a++;
        }
    }
}

void draw_skinned_rect(BITMAP*dst, LexSkinnedRect *skin,
        int x, int y, int w, int h)
{
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
    delete gui;

    gui_shutdown();
}

void gui_shutdown(void) {
    int a, b;

    /* Button */
    for (a = 0; a < 3; a++) {
        for (b = 0; b < 9 ; b++) {
            destroy_bitmap(gui_skin.button.background[a].grid[b]);
        }
    }

    /* Slider */
    for (a = 0; a < 3; a++) {
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

    for (a = 0; a < GUI_BMP_COUNT; a++) {
        destroy_bitmap(gui__repository[a]);
    }
}

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
    if (msg == MSG_DRAW) {
        gui_text(gui_bitmap, (char *)d->dp, d->x, d->y, d->fg, FALSE);
    }
    return D_O_K;
}

int tmw_button_proc(int msg, DIALOG *d, int c)
{
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
        /*
        if (msg == MSG_CLICK) {
            if (d->d1 == 1) ((int)d->dp2) + 1;
            else if (d->d1 == 2) ((int)d->dp2) - 1;
        }
        */

        ret =  d_button_proc(msg,d,c);
    }
    return ret;
}


int tmw_dialog_proc(int msg, DIALOG *d, int c) {
    int x, y;

    switch (msg) {
        case MSG_CLICK:
            if (mouse_y<d->y + gui_skin.dialog.bg.grid[1]->h) {
                d->d1 = mouse_x - d->x;
                d->d2 = mouse_y - d->y;
            }
            break;
        case MSG_DRAW:
            if((mouse_b & 1) && (d->d1 >= 0) && (d->d2 >= 0)) {
                x = mouse_x - d->d1;
                y = mouse_y - d->d2;
                if (x < 15) {
                    x = 0;
                    position_mouse(d->d1, mouse_y);
                }
                if (y < 15) {
                    y = 0;
                    position_mouse(mouse_x, d->d2);
                }
                if (x + d->w >= SCREEN_W - 15) {
                    x = SCREEN_W - d->w;
                    position_mouse(x+d->d1, mouse_y);
                }
                if (y + d->h >= SCREEN_H - 15) {
                    y = SCREEN_H - d->h;
                    position_mouse(mouse_x, y + d->d2);
                }
                position_dialog(d, x, y);
            } else {
                d->d1 = -1;
                d->d2 = -1;
            }
            draw_skinned_rect(gui_bitmap, &gui_skin.dialog.bg,
                    d->x, d->y, d->w, d->h);

            textprintf_centre_ex(gui_bitmap, font,
                    d->x + d->w / 2,
                    d->y + (gui_skin.dialog.bg.grid[1]->h - text_height(font)) / 2,
                    d->fg, -1, "%s", (char*)d->dp);

            break;
    }
    return D_O_K;
}

void ok(const char *title, const char *message) {
    DIALOG alert_dialog[] = {
        /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
        { tmw_dialog_proc,     0,    0,  0,     60,    0,  -1,    0,    0,          0,                      0,    (void *)title,    NULL, NULL  },
        { tmw_text_proc,       2,   22,  0,      0,    0,   0,    0,    0,          0,                      0,    (void *)message,  NULL, NULL  },
        { tmw_button_proc,     0,   40,  44,    18,    0,  -1,    'o',  D_EXIT,    -1,                      0,    (char *)"&Ok",    NULL, NULL  },
        { NULL,                0,    0,   0,     0,    0,   0,    0,    0,          0,                      0,    NULL,             NULL, NULL  }
    };

    BITMAP *temp = gui_bitmap;
    gui_bitmap = screen;
    show_mouse(screen);
    alert_dialog[0].w = text_length(font, message) + 4;
    alert_dialog[1].w = text_length(font, message);
    alert_dialog[1].h = text_height(font);
    alert_dialog[2].x = text_length(font, message) / 2 - 22;
    position_dialog(alert_dialog, 400 - alert_dialog[0].w / 2, 270);
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
        { NULL,                0,    0,   0,     0,    0,   0,    0,    0,          0,                      0,    NULL,             NULL, NULL  }
    };

    BITMAP *temp = gui_bitmap;
    gui_bitmap = screen;
    show_mouse(screen);
    int width = text_length(font, message) + 4;
    if (width < 100) width = 100;
    alert_dialog[0].w = width;
    alert_dialog[1].w = text_length(font, message);
    alert_dialog[1].h = text_height(font);
    alert_dialog[2].x = width / 2 - 46;
    alert_dialog[2].x = width / 2 + 2;
    position_dialog(alert_dialog, 400 - width / 2, 270);
    ret = do_dialog(alert_dialog, 3);
    show_mouse(NULL);
    gui_bitmap = temp;
    return ret - 2;
}
