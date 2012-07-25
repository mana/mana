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

#include "setup.h"

#include "main.h"

#include "gui/setup_audio.h"
#include "gui/setup_colors.h"
#include "gui/setup_joystick.h"
#include "gui/setup_keyboard.h"
#include "gui/setup_players.h"
#include "gui/setup_video.h"
#include "gui/setup_interface.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/tabbedarea.h"

#include "utils/dtor.h"
#include "utils/gettext.h"

extern Window *statusWindow;

Setup::Setup():
    Window(_("Setup"))
{
    setCloseButton(true);
    int width = 395;
    int height = 300;
    setContentSize(width, height);

    static const char *buttonNames[] = {
        N_("Apply"), N_("Cancel"), N_("Reset Windows"), 0
    };
    int x = width;
    for (const char **curBtn = buttonNames; *curBtn; ++curBtn)
    {
        Button *btn = new Button(gettext(*curBtn), *curBtn, this);
        x -= btn->getWidth() + 5;
        btn->setPosition(x, height - btn->getHeight() - 5);
        add(btn);

        // Store this button, as it needs to be enabled/disabled
        if (!strcmp(*curBtn, "Reset Windows"))
            mResetWindows = btn;
    }

    TabbedArea *panel = new TabbedArea;
    panel->setDimension(gcn::Rectangle(5, 5, width - 10, height - 40));

    mTabs.push_back(new Setup_Video);
    mTabs.push_back(new Setup_Audio);
    mTabs.push_back(new Setup_Interface);
    mTabs.push_back(new Setup_Joystick);
    mTabs.push_back(new Setup_Keyboard);
    mTabs.push_back(new Setup_Colors);
    mTabs.push_back(new Setup_Players);

    for (std::list<SetupTab*>::iterator i = mTabs.begin(), i_end = mTabs.end();
         i != i_end; ++i)
    {
        SetupTab *tab = *i;
        panel->addTab(tab->getName(), tab);
    }

    add(panel);

    Label *version = new Label(FULL_VERSION);
    version->setPosition(9, height - version->getHeight() - 9);
    add(version);

    center();

    setInGame(false);
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

        for (std::list<Window*>::iterator it = mWindowsToReset.begin();
                it != mWindowsToReset.end(); it++)
        {
            (*it)->resetToDefaultSize();
        }
    }
}

void Setup::setInGame(bool inGame)
{
    mResetWindows->setEnabled(inGame);
}

void Setup::registerWindowForReset(Window *window)
{
    mWindowsToReset.push_back(window);
}

Setup *setupWindow;
