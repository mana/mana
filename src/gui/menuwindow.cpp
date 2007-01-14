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
#include "windowcontainer.h"

extern Window *setupWindow, *inventoryWindow, *equipmentWindow,
       *skillDialog, *statusWindow;

namespace {
    struct MenuWindowListener : public gcn::ActionListener
    {
        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event);
    } listener;
}

MenuWindow::MenuWindow():
    Window("")
{
    setResizable(false);
    setWindowName("Menu");
    setMovable(false);
    setTitleBarHeight(0);

    // Buttons
    // ------------
    const char *buttonNames[] = {
        "Status", "Equipment", "Inventory", "Skills", "Setup", 0
    };
    int x = 0, y = 3, h = 0;

    for (const char **curBtn = buttonNames; *curBtn; curBtn++) {
        gcn::Button *btn = new Button(*curBtn, *curBtn, &listener);
        btn->setPosition(x, y);
        add(btn);
        x += btn->getWidth() + 3;
        h = btn->getHeight();
    }

    setDefaultSize((windowContainer->getWidth() - x - 2), 0, x, (y + h));
}

void MenuWindow::draw(gcn::Graphics *graphics)
{
    drawChildren(graphics);
}


void MenuWindowListener::action(const gcn::ActionEvent &event)
{
    Window *window = NULL;
    if (event.getId() == "Status")
    {
        window = statusWindow;
    }
    else if (event.getId() == "Equipment")
    {
        window = equipmentWindow;
    }
    else if (event.getId() == "Inventory")
    {
        window = inventoryWindow;
    }
    else if (event.getId() == "Skills")
    {
        window = skillDialog;
    }
    else if (event.getId() == "Setup")
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
