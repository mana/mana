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

#include "button.h"

Button::Button(const std::string& caption):
    gcn::Button(caption)
{
    mouseDown = false;
    keyDown = false;
    setBorderSize(0);
}

void Button::draw(gcn::Graphics* graphics) {
    int x, y;
    int mode;
    int offset = 0;

    getAbsolutePosition(x, y);

    //printf("draw - %d,%d\n", x, y);

    if (false /*disabled*/) {
        mode = 3;
    }
    else if (hasMouse() && mouseDown || keyDown) {
        mode = 2;
        offset = 1;
    }
    else if (hasMouse()) {
        mode = 1;
    }
    else {
        mode = 0;
    }

    draw_skinned_rect(gui_bitmap, &gui_skin.button.background[mode],
            x, y, getWidth(), getHeight());

    int rtm = alfont_text_mode(-1);
    gui_text(gui_bitmap, getCaption().c_str(),
            x + 2 + offset, y + 4 + offset,
            gui_skin.button.textcolor[mode], FALSE);
    alfont_text_mode(rtm);
}

void Button::lostFocus() {
    mouseDown = false;
    keyDown = false;
}

void Button::mousePress(int x, int y, int button) {
    if (button == gcn::MouseInput::LEFT && hasMouse()) {
        mouseDown = true;
    }
}

void Button::mouseRelease(int x, int y, int button) {
    if (button == gcn::MouseInput::LEFT) {
        mouseDown = false;
    }
}

void Button::keyPress(const gcn::Key& key) {
    if (key.getValue() == gcn::Key::ENTER ||
        key.getValue() == gcn::Key::SPACE)
    {
        keyDown = true;
    }
    mouseDown = false;
}

void Button::keyRelease(const gcn::Key& key) {
    if ((key.getValue() == gcn::Key::ENTER ||
         key.getValue() == gcn::Key::SPACE) && keyDown)
    {
        keyDown = false;
        generateAction();
    }
}

