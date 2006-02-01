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

#include "menuwindow.h"

#include <string>

#include <guichan/actionlistener.hpp>

#include "button.h"

#include "../graphics.h"

extern Graphics *graphics;
extern Window *setupWindow, *inventoryWindow, *equipmentWindow,
       *skillDialog, *statusWindow;

class MenuWindowListener : public gcn::ActionListener
{
         /**
         * Called when receiving actions from widget.
         */
        void action(const std::string& eventId);
} menuWindowListener;

MenuWindow::MenuWindow():
    Window("")
{
    setResizable(false);
    setWindowName("Menu");
    setMovable(false);
    setTitleBarHeight(0);

    // Buttons
    // ------------
    char *buttonNames[] = {
        "Status", "Equipment", "Inventory", "Skills", "Setup", 0
    };
    int x = 0, y = 3, h = 0;

    for (char **curBtn = buttonNames; *curBtn; curBtn++) {
        gcn::Button *btn = new Button(*curBtn);
        btn->setEventId(*curBtn);
        btn->addActionListener(&menuWindowListener);
        btn->setPosition(x, y);
        add(btn);
        x += btn->getWidth() + 3;
        h = btn->getHeight();
    }

    setDefaultSize((graphics->getWidth() - x - 2), 0, x, (y + h));
}

void MenuWindow::draw(gcn::Graphics *g)
{
    Window::drawContent(g);
}


void MenuWindowListener::action(const std::string& eventId)
{
    Window *window = NULL;
    if (eventId == "Status")
    {
        window = statusWindow;
    }
    else if (eventId == "Equipment")
    {
        window = equipmentWindow;
    }
    else if (eventId == "Inventory")
    {
        window = inventoryWindow;
    }
    else if (eventId == "Skills")
    {
        window = skillDialog;
    }
    else if (eventId == "Setup")
    {
        window = setupWindow;
    }

    if (window) {
        window->setVisible(!window->isVisible());
        if (window->isVisible()) {
            window->requestMoveToTop();
        }
    }
}
