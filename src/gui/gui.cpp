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
#include "window.h"
#include "windowcontainer.h"

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
DATAFILE *gui_gfx;


// Guichan stuff
Gui *gui;
gcn::AllegroGraphics *guiGraphics;     // Graphics driver
WindowContainer *guiTop;               // The top container


Gui::Gui(Graphics *graphics)
{
    // Set graphics
    guiGraphics = graphics;

    // Set input
    guiInput = new AllegroInput();

    // Set image loader
    imageLoader = new gcn::AllegroImageLoader();
    gcn::Image::setImageLoader(imageLoader);

    // Initialize top GUI widget
    guiTop = new WindowContainer();
    guiTop->setDimension(gcn::Rectangle(0, 0, SCREEN_W, SCREEN_H));
    guiTop->setOpaque(false);
    Window::setWindowContainer(guiTop);

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

    // Draw the mouse
    draw_sprite(buffer, mouse_sprite, mouse_x, mouse_y);

    guiGraphics->_endDraw();
}

void Gui::focusNone()
{
    focusHandler->focusNone();
}


void init_gui(Graphics *graphics) {
    gui = new Gui(graphics);

    // TODO: Remove Allegro config file usage from GUI look
    gui_load_skin("data/Skin/aqua.skin");
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
    loadTextboxSkin();
    loadListboxSkin();
    loadDialogSkin();
    loadBarSkin();
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

    for (a = 0; a < GUI_BMP_COUNT; a++) {
        destroy_bitmap(gui__repository[a]);
    }
}
