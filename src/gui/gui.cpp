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
#include "window.h"
#include "windowcontainer.h"

// Guichan stuff
Gui *gui;
Graphics *guiGraphics;                 // Graphics driver
gcn::SDLInput *guiInput;               // GUI input
WindowContainer *guiTop;               // The top container

Gui::Gui(Graphics *graphics):
    topHasMouse(false)
{
    // Set graphics
    guiGraphics = graphics;

    // Set input
    guiInput = new gcn::SDLInput();

    // Set image loader
    imageLoader = new gcn::SDLImageLoader();
    gcn::Image::setImageLoader(imageLoader);

    // Initialize top GUI widget
    guiTop = new WindowContainer();
    guiTop->setDimension(gcn::Rectangle(0, 0, screen->w, screen->h));
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
    delete focusHandler;
}

void Gui::logic()
{
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
    //draw_sprite(buffer, mouse_sprite, mouse_x, mouse_y);

    guiGraphics->_endDraw();
}

void Gui::focusNone()
{
    focusHandler->focusNone();
}

void init_gui(Graphics *graphics) {
    gui = new Gui(graphics);
}

void gui_exit() {
    delete gui;
}
