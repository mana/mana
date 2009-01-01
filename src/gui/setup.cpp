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

#include <algorithm>

#include "setup.h"

#include "button.h"
#include "setup_audio.h"
#include "setup_joystick.h"
#include "setup_video.h"
#include "setup_keyboard.h"
#include "setup_players.h"
#include "tabbedcontainer.h"

#include "../utils/dtor.h"
#include <iostream>

extern Window *statusWindow;
extern Window *minimap;
extern Window *chatWindow;
extern Window *inventoryWindow;
extern Window *equipmentWindow;
extern Window *helpWindow;
extern Window *skillDialog;
extern Window *itemShortcutWindow;
extern Window *tradeWindow;

Setup::Setup():
    Window("Setup")
{
    setCloseButton(true);
    int width = 310;
    int height = 265;
    setContentSize(width, height);

    const char *buttonNames[] = {
        "Apply", "Cancel", "Reset Windows", 0
    };
    int x = width;
    for (const char **curBtn = buttonNames; *curBtn; ++curBtn) {
        Button *btn = new Button(*curBtn, *curBtn, this);
        x -= btn->getWidth() + 5;
        btn->setPosition(x, height - btn->getHeight() - 5);
        add(btn);

        // Disable this button when the windows aren't created yet
        if (!strcmp(*curBtn, "Reset Windows"))
            btn->setEnabled(statusWindow != NULL);
    }

    TabbedContainer *panel = new TabbedContainer();
    panel->setDimension(gcn::Rectangle(5, 5, width, height - 40));
    panel->setOpaque(false);

    SetupTab *tab;

    tab = new Setup_Video();
    panel->addTab(tab, "Video");
    mTabs.push_back(tab);

    tab = new Setup_Audio();
    panel->addTab(tab, "Audio");
    mTabs.push_back(tab);

    tab = new Setup_Joystick();
    panel->addTab(tab, "Joystick");
    mTabs.push_back(tab);

    tab = new Setup_Keyboard();
    panel->addTab(tab, "Keyboard");
    mTabs.push_back(tab);

    tab = new Setup_Players();
    panel->addTab(tab, "Players");
    mTabs.push_back(tab);

    add(panel);

    setLocationRelativeTo(getParent());
}

Setup::~Setup()
{
    delete_all(mTabs);
}

void Setup::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "Apply")
    {
        setVisible(false);
        for_each(mTabs.begin(), mTabs.end(), std::mem_fun(&SetupTab::apply));
    }
    else if (event.getId() == "Cancel")
    {
        setVisible(false);
        for_each(mTabs.begin(), mTabs.end(), std::mem_fun(&SetupTab::cancel));
    }
    else if (event.getId() == "Reset Windows")
    {
        // Bail out if this action happens to be activated before the windows
        // are created (though it should be disabled then)
        if (!statusWindow)
            return;

        statusWindow->resetToDefaultSize();
        minimap->resetToDefaultSize();
        chatWindow->resetToDefaultSize();
        inventoryWindow->resetToDefaultSize();
        equipmentWindow->resetToDefaultSize();
        helpWindow->resetToDefaultSize();
        skillDialog->resetToDefaultSize();
        itemShortcutWindow->resetToDefaultSize();
        tradeWindow->resetToDefaultSize();
    }
}
