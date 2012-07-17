/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

    addButton(":-)", x, h, "button-icon-smilies.png");
    addButton(N_("Status"), x, h, "button-icon-status.png",
              KeyboardConfig::KEY_WINDOW_STATUS);
    addButton(N_("Inventory"), x, h, "button-icon-inventory.png",
              KeyboardConfig::KEY_WINDOW_INVENTORY);
    addButton(N_("Equipment"), x, h, "button-icon-equipment.png",
              KeyboardConfig::KEY_WINDOW_EQUIPMENT);

    if (skillDialog->hasSkills())
        addButton(N_("Skills"), x, h, "button-icon-skills.png",
                  KeyboardConfig::KEY_WINDOW_SKILL);

    addButton(N_("Specials"), x, h, "button-icon-specials.png");

    addButton(N_("Social"), x, h, "button-icon-social.png",
        KeyboardConfig::KEY_WINDOW_SOCIAL);
    addButton(N_("Shortcuts"), x, h, "button-icon-shortcut.png",
        KeyboardConfig::KEY_WINDOW_SHORTCUT);
    addButton(N_("Setup"), x, h, "button-icon-setup.png",
        KeyboardConfig::KEY_WINDOW_SETUP);

    setDimension(gcn::Rectangle(graphics->getWidth() - x, 3,
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
    else if (event.getId() == "Shortcuts")
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

static std::string createShortcutCaption(const std::string& text,
                                       KeyboardConfig::KeyAction key)
{
    std::string caption = gettext(text.c_str());
    if (key != KeyboardConfig::KEY_NO_VALUE)
    {
        caption += " (";
        caption += SDL_GetKeyName((SDLKey) keyboard.getKeyValue(key));
        caption += ")";
    }
    return caption;
}

void WindowMenu::addButton(const std::string& text, int &x, int &h,
                           const std::string& iconPath,
                           KeyboardConfig::KeyAction key)
{
    Button *btn = new Button("", text, this);
    if (!iconPath.empty() && btn->setButtonIcon(iconPath))
    {
        btn->setButtonPopupText(createShortcutCaption(text, key));
    }
    else
    {
        btn->setCaption(gettext(text.c_str()));
        btn->setButtonPopupText(createShortcutCaption("", key));
    }

    btn->setPosition(x, 0);
    add(btn);
    x += btn->getWidth() + 3;
    h = std::max(h, btn->getHeight());
}

void WindowMenu::updatePopUpCaptions()
{
    for (WidgetList::iterator it = mWidgets.begin(); it != mWidgets.end(); ++it)
    {
        Button *button = dynamic_cast<Button*> (*it);
        if (!button)
            continue;

        std::string eventId = button->getActionEventId();
        if (eventId == "Status")
        {
            button->setButtonPopupText(createShortcutCaption("Status",
                                        KeyboardConfig::KEY_WINDOW_STATUS));
        }
        else if (eventId == "Equipment")
        {
            button->setButtonPopupText(createShortcutCaption("Equipment",
                                        KeyboardConfig::KEY_WINDOW_EQUIPMENT));
        }
        else if (eventId == "Inventory")
        {
            button->setButtonPopupText(createShortcutCaption("Inventory",
                                        KeyboardConfig::KEY_WINDOW_INVENTORY));
        }
        else if (eventId == "Skills")
        {
            button->setButtonPopupText(createShortcutCaption("Skills",
                                            KeyboardConfig::KEY_WINDOW_SKILL));
        }
        else if (eventId == "Social")
        {
            button->setButtonPopupText(createShortcutCaption("Social",
                                        KeyboardConfig::KEY_WINDOW_SOCIAL));
        }
        else if (eventId == "Shortcuts")
        {
            button->setButtonPopupText(createShortcutCaption("Shortcuts",
                                        KeyboardConfig::KEY_WINDOW_SHORTCUT));
        }
        else if (eventId == "Setup")
        {
            button->setButtonPopupText(createShortcutCaption("Setup",
                                            KeyboardConfig::KEY_WINDOW_SETUP));
        }
    }
}
