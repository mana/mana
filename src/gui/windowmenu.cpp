/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/windowmenu.h"

#include "graphics.h"

#include "gui/emotepopup.h"
#include "gui/skilldialog.h"
#include "gui/specialswindow.h"

#include "gui/widgets/button.h"
#include "gui/widgets/window.h"
#include "gui/widgets/windowcontainer.h"

#include "net/net.h"
#include "net/playerhandler.h"

#include "utils/gettext.h"

#include <string>

extern Window *equipmentWindow;
extern Window *inventoryWindow;
extern Window *itemShortcutWindow;
extern Window *setupWindow;
extern Window *statusWindow;
extern Window *socialWindow;

WindowMenu::WindowMenu():
    mEmotePopup(0)
{
    int x = 0, h = 0;

    addButton(":-)", x, h);
    addButton(N_("Status"), x, h);
    addButton(N_("Equipment"), x, h);
    addButton(N_("Inventory"), x, h);

    if (skillDialog->hasSkills())
        addButton(N_("Skills"), x, h);

    // if (specialsWindow->hasSpecials())
        addButton(N_("Specials"), x, h);

    addButton(N_("Social"), x, h, "button-icon-social.png");
    addButton(N_("Shortcut"), x, h);
    addButton(N_("Setup"), x, h, "button-icon-setup.png");

    setDimension(gcn::Rectangle(graphics->getWidth() - x - 3, 3,
                                x - 3, h));
    setVisible(true);
}

WindowMenu::~WindowMenu()
{
    delete mEmotePopup;
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
    else if (event.getId() == "Specials")
    {
        window = specialsWindow;
    }
    else if (event.getId() == "Social")
    {
        window = socialWindow;
    }
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

void WindowMenu::addButton(const std::string& text, int &x, int &h,
                           const std::string& iconPath)
{
    Button *btn = new Button("", text, this);
    if (!iconPath.empty() && btn->setButtonIcon(iconPath))
    {
        btn->setButtonPopupText(gettext(text.c_str()));
    }
    else
    {
        btn->setCaption(gettext(text.c_str()));
    }

    btn->setPosition(x, 0);
    add(btn);
    x += btn->getWidth() + 3;
    h = std::max(h, btn->getHeight());
}
