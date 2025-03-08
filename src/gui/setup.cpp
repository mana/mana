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
#include "gui/widgets/layout.h"
#include "gui/widgets/tabbedarea.h"

#include "utils/gettext.h"

extern Window *statusWindow;

Setup::Setup():
    Window(_("Setup"))
{
    setWindowName("Setup");
    setCloseButton(true);
    setResizable(true);

    mResetWindows = new Button(_("Reset Windows"), "Reset Windows", this);
    mResetWindows->setEnabled(false);

    place(2, 5, mResetWindows);
    place(3, 5, new Button(_("Apply"), "Apply", this));
    place(4, 5, new Button(_("Cancel"), "Cancel", this));

    auto *panel = new TabbedArea;

    mTabs.emplace_back(new Setup_Video);
    mTabs.emplace_back(new Setup_Audio);
    mTabs.emplace_back(new Setup_Interface);
    mTabs.emplace_back(new Setup_Joystick);
    mTabs.emplace_back(new Setup_Keyboard);
    mTabs.emplace_back(new Setup_Colors);
    mTabs.emplace_back(new Setup_Players);

    for (auto &tab : mTabs)
        panel->addTab(tab->getName(), tab.get());

    place(0, 0, panel, 5, 5);
    place(0, 5, new Label(FULL_VERSION));

    // Determine minimum width by layout
    int width = 0, height = 350;
    getLayout().reflow(width, height);

    setMinWidth(width + 2 * getPadding());
    setMinHeight(height + getPadding() + getTitleBarHeight());
    setDefaultSize(395, 360 + getPadding() + getTitleBarHeight(), WindowAlignment::Center);
    loadWindowState();
}

Setup::~Setup() = default;

void Setup::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "Apply")
    {
        setVisible(false);

        for (auto &tab : mTabs)
            tab->apply();
    }
    else if (event.getId() == "Cancel")
    {
        setVisible(false);

        for (auto &tab : mTabs)
            tab->cancel();
    }
    else if (event.getId() == "Reset Windows")
    {
        for (auto &window : mWindowsToReset)
            window->resetToDefaultSize();
    }
}

void Setup::registerWindowForReset(Window *window)
{
    mWindowsToReset.push_back(window);
    mResetWindows->setEnabled(true);
}

void Setup::clearWindowsForReset()
{
    mWindowsToReset.clear();
    mResetWindows->setEnabled(false);
}

Setup *setupWindow;
