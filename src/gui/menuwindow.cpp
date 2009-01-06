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

#include <string>

#include <guichan/actionlistener.hpp>

#include "button.h"
#include "menuwindow.h"
#include "windowcontainer.h"

extern Window *chatWindow;
extern Window *equipmentWindow;
extern Window *inventoryWindow;
extern Window *itemShortcutWindow;
extern Window *smileyWindow;
extern Window *setupWindow;
extern Window *skillDialog;
extern Window *statusWindow;

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
    const char *buttonNames[] =
    {
        "Chat",
        "Status",
        "Equipment",
        "Inventory",
        "Skills",
        "Shortcut",
        "Emote",
        "Setup",
        0
    };
    int x = 0, h = 0;

    for (const char **curBtn = buttonNames; *curBtn; curBtn++)
    {
        gcn::Button *btn = new Button(*curBtn, *curBtn, &listener);
        btn->setPosition(x, 0);
        add(btn);
        x += btn->getWidth() + 3;
        h = btn->getHeight();
    }

    setPosition(windowContainer->getWidth() - x - 3, 3);
    setContentSize(x - 3, h);
}

void MenuWindow::draw(gcn::Graphics *graphics)
{
    drawChildren(graphics);
}


void MenuWindowListener::action(const gcn::ActionEvent &event)
{
    Window *window = NULL;

    if (event.getId() == "Chat")
    {
        window = chatWindow;
    }
    else if (event.getId() == "Status")
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
    else if (event.getId() == "Shortcut")
    {
        window = itemShortcutWindow;
    }
    else if (event.getId() == "Emote")
    {
        window = smileyWindow;
    }
    else if (event.getId() == "Setup")
    {
        window = setupWindow;
    }

    if (window)
    {
        window->setVisible(!window->isVisible());
        if (window->isVisible())
        {
            window->requestMoveToTop();
        }
    }
}
