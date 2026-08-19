/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "gui/abilitieswindow.h"
#include "gui/emotepopup.h"
#include "gui/questswindow.h"
#include "gui/skilldialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/window.h"
#include "gui/widgets/windowcontainer.h"

#include "net/net.h"
#include "net/playerhandler.h"

#include "resources/questdb.h"

#include "utils/gettext.h"

#include <string>

extern Window *equipmentWindow;
extern Window *inventoryWindow;
extern Window *itemShortcutWindow;
extern Window *setupWindow;
extern Window *statusWindow;
extern Window *socialWindow;

WindowMenu::WindowMenu()
{
    int x = 0, h = 0;

    addButton(":-)", x, h, "button-icon-smilies.png");
    mEmoteButton = dynamic_cast<Button *>(mWidgets.back());
    mEmoteButton->setToggle(true);
    addWindowButton(N_("Status"), statusWindow, x, h,
                    "button-icon-status.png",
                    KeyboardConfig::KEY_WINDOW_STATUS);
    addWindowButton(N_("Inventory"), inventoryWindow, x, h,
                    "button-icon-inventory.png",
                    KeyboardConfig::KEY_WINDOW_INVENTORY);
    addWindowButton(N_("Equipment"), equipmentWindow, x, h,
                    "button-icon-equipment.png",
                    KeyboardConfig::KEY_WINDOW_EQUIPMENT);

    if (skillDialog->hasSkills())
        addWindowButton(N_("Skills"), skillDialog, x, h,
                        "button-icon-skills.png",
                        KeyboardConfig::KEY_WINDOW_SKILL);

    if (abilitiesWindow->hasAbilities())
        addWindowButton(N_("Abilities"), abilitiesWindow, x, h,
                        "button-icon-abilities.png");

    if (QuestDB::hasQuests())
        addWindowButton(N_("Quests"), questsWindow, x, h,
                        "button-icon-quests.png");

    addWindowButton(N_("Social"), socialWindow, x, h,
                    "button-icon-social.png",
                    KeyboardConfig::KEY_WINDOW_SOCIAL);
    addWindowButton(N_("Shortcuts"), itemShortcutWindow, x, h,
                    "button-icon-shortcut.png",
                    KeyboardConfig::KEY_WINDOW_SHORTCUT);
    addWindowButton(N_("Setup"), setupWindow, x, h,
                    "button-icon-setup.png",
                    KeyboardConfig::KEY_WINDOW_SETUP);

    setDimension(gcn::Rectangle(graphics->getWidth() - x, 3,
                                x - 3, h));
    setVisible(true);
}

WindowMenu::~WindowMenu()
{
    for (auto &[window, btn] : mWindowButtons)
        window->removeWidgetListener(this);

    delete mEmotePopup;
}

void WindowMenu::action(const gcn::ActionEvent &event)
{
    Window *window = nullptr;

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
            mEmotePopup = nullptr;
        }
    }
    for (auto &[win, btn] : mWindowButtons)
    {
        if (event.getId() == btn->getActionEventId())
        {
            window = win;
            break;
        }
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
        int emoteId = mEmotePopup->getSelectedEmoteId();
        if (emoteId != -1)
            Net::getPlayerHandler()->emote(emoteId);

        windowContainer->scheduleDelete(mEmotePopup);
        mEmotePopup = nullptr;
        mEmoteButton->setSelected(false);
    }
}

static std::string createShortcutCaption(const std::string &text,
                                         KeyboardConfig::KeyAction key)
{
    std::string caption = gettext(text.c_str());

    if (key != KeyboardConfig::KEY_NO_VALUE)
    {
        auto keyValue = keyboard.getKeyValue(key);
        if (keyValue > 0)
        {
            caption += " (";
            caption += SDL_GetKeyName(keyValue);
            caption += ")";
        }
    }

    return caption;
}

void WindowMenu::addButton(const std::string &text, int &x, int &h,
                           const std::string &iconPath,
                           KeyboardConfig::KeyAction key)
{
    auto *btn = new Button(std::string(), text, this);
    if (!iconPath.empty() && btn->setButtonIcon(iconPath))
    {
        btn->setButtonPopupText(createShortcutCaption(text, key));
    }
    else
    {
        btn->setCaption(gettext(text.c_str()));
        btn->setButtonPopupText(createShortcutCaption(std::string(), key));
    }

    btn->setPosition(x, 0);
    add(btn);
    x += btn->getWidth() + 3;
    h = std::max(h, btn->getHeight());
}

void WindowMenu::addWindowButton(const std::string &text, Window *window,
                                  int &x, int &h,
                                  const std::string &iconPath,
                                  KeyboardConfig::KeyAction key)
{
    addButton(text, x, h, iconPath, key);
    auto *btn = dynamic_cast<Button *>(mWidgets.back());
    btn->setToggle(true);
    btn->setSelected(window->isVisible());
    mWindowButtons[window] = btn;
    window->addWidgetListener(this);
}

void WindowMenu::widgetHidden(const gcn::Event &event)
{
    auto it = mWindowButtons.find(static_cast<Window *>(event.getSource()));
    if (it != mWindowButtons.end())
        it->second->setSelected(false);
}

void WindowMenu::widgetShown(const gcn::Event &event)
{
    auto it = mWindowButtons.find(static_cast<Window *>(event.getSource()));
    if (it != mWindowButtons.end())
        it->second->setSelected(true);
}

void WindowMenu::updatePopUpCaptions()
{
    for (auto &widget : mWidgets)
    {
        auto *button = dynamic_cast<Button*>(widget);
        if (!button)
            continue;

        const std::string &eventId = button->getActionEventId();
        if (eventId == "Status")
        {
            button->setButtonPopupText(createShortcutCaption(eventId,
                                        KeyboardConfig::KEY_WINDOW_STATUS));
        }
        else if (eventId == "Equipment")
        {
            button->setButtonPopupText(createShortcutCaption(eventId,
                                        KeyboardConfig::KEY_WINDOW_EQUIPMENT));
        }
        else if (eventId == "Inventory")
        {
            button->setButtonPopupText(createShortcutCaption(eventId,
                                        KeyboardConfig::KEY_WINDOW_INVENTORY));
        }
        else if (eventId == "Skills")
        {
            button->setButtonPopupText(createShortcutCaption(eventId,
                                            KeyboardConfig::KEY_WINDOW_SKILL));
        }
        else if (eventId == "Quests")
        {
            button->setButtonPopupText(
                createShortcutCaption(eventId, KeyboardConfig::KEY_WINDOW_QUESTS));
        }
        else if (eventId == "Social")
        {
            button->setButtonPopupText(createShortcutCaption(eventId,
                                        KeyboardConfig::KEY_WINDOW_SOCIAL));
        }
        else if (eventId == "Shortcuts")
        {
            button->setButtonPopupText(createShortcutCaption(eventId,
                                        KeyboardConfig::KEY_WINDOW_SHORTCUT));
        }
        else if (eventId == "Setup")
        {
            button->setButtonPopupText(createShortcutCaption(eventId,
                                            KeyboardConfig::KEY_WINDOW_SETUP));
        }
    }
}
