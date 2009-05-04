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

#include "gui/windowmenu.h"

#include "gui/emotepopup.h"

#include "gui/widgets/button.h"
#include "gui/widgets/window.h"
#include "gui/widgets/windowcontainer.h"

#include "graphics.h"

#include "net/net.h"
#include "net/playerhandler.h"

#include "utils/gettext.h"

#include <string>

extern Window *equipmentWindow;
extern Window *inventoryWindow;
extern Window *itemShortcutWindow;
extern Window *setupWindow;
extern Window *skillDialog;
extern Window *statusWindow;
#ifdef TMWSERV_SUPPORT
extern Window *buddyWindow;
extern Window *guildWindow;
extern Window *magicDialog;
#endif


WindowMenu::WindowMenu():
    mEmotePopup(0)
{
    // Buttons
    static const char *buttonNames[] =
    {
        ":-)",
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
        N_("Setup"),
        0
    };
    int x = 0, h = 0;

    for (const char **curBtn = buttonNames; *curBtn; curBtn++)
    {
        gcn::Button *btn = new Button(gettext(*curBtn), *curBtn, this);
        btn->setPosition(x, 0);
        add(btn);
        x += btn->getWidth() + 3;
        h = btn->getHeight();
    }

    setDimension(gcn::Rectangle(graphics->getWidth() - x - 3, 3,
                                x - 3, h));
    setVisible(true);
}

WindowMenu::~WindowMenu()
{
}

void WindowMenu::action(const gcn::ActionEvent &event)
{
    Window *window = 0;

    if (event.getId() == ":-)")
    {
        if (!mEmotePopup)
        {
            const gcn::Widget *s = event.getSource();
            const gcn::Rectangle &r = s->getDimension();
            const int parentX = s->getParent()->getX();

            mEmotePopup = new EmotePopup;
            const int offset = (r.width - mEmotePopup->getWidth()) / 2;
            mEmotePopup->setPosition(parentX + r.x + offset,
                                     r.y + r.height + 5);

            mEmotePopup->addSelectionListener(this);
        }
        else
        {
            windowContainer->scheduleDelete(mEmotePopup);
            mEmotePopup = 0;
        }
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

void WindowMenu::valueChanged(const gcn::SelectionEvent &event)
{
    if (event.getSource() == mEmotePopup)
    {
        int emote = mEmotePopup->getSelectedEmote();
        if (emote)
            Net::getPlayerHandler()->emote(emote);

        windowContainer->scheduleDelete(mEmotePopup);
        mEmotePopup = 0;
    }
}
