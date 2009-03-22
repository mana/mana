/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string>

#include <guichan/actionlistener.hpp>

#include "button.h"
#include "menuwindow.h"
#include "windowcontainer.h"

#include "../utils/gettext.h"

extern Window *chatWindow;
extern Window *equipmentWindow;
extern Window *inventoryWindow;
extern Window *itemShortcutWindow;
extern Window *emoteWindow;
extern Window *setupWindow;
extern Window *skillDialog;
extern Window *statusWindow;
#ifdef TMWSERV_SUPPORT
extern Window *buddyWindow;
extern Window *guildWindow;
extern Window *magicDialog;
#endif

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
    Window()
{
    setResizable(false);
    setWindowName("Menu");
    setMovable(false);
    setTitleBarHeight(0);

    // Buttons
    static const char *buttonNames[] =
    {
        N_("Chat"),
        N_("Status"),
        N_("Equipment"),
        N_("Inventory"),
        N_("Skills"),
#ifdef TMWSERV_SUPPORT
        N_("Magic"),
        N_("Guilds"),
        N_("Buddys"),
#endif
        N_("Shortcut"),
        N_("Emote"),
        N_("Setup"),
        0
    };
    int x = 0, h = 0;

    for (const char **curBtn = buttonNames; *curBtn; curBtn++)
    {
        gcn::Button *btn = new Button(gettext(*curBtn), *curBtn, &listener);
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
#ifdef TMWSERV_SUPPORT
    else if (event.getId() == "Magic")
    {
        window = magicDialog;
    }
    else if (event.getId() == "Guilds")
    {
        window = guildWindow;
    }
    else if (event.getId() == "Buddys")
    {
        window = buddyWindow;
    }
#endif
    else if (event.getId() == "Shortcut")
    {
        window = itemShortcutWindow;
    }
    else if (event.getId() == "Emote")
    {
        window = emoteWindow;
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
