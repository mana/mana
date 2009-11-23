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

#include "gui/quitdialog.h"

#include "main.h"

#include "gui/widgets/layout.h"
#include "gui/widgets/button.h"
#include "gui/widgets/radiobutton.h"

#include "net/charhandler.h"
#include "net/net.h"

#include "utils/gettext.h"

#include <assert.h>

QuitDialog::QuitDialog(QuitDialog** pointerToMe):
    Window(_("Quit"), true, NULL), mMyPointer(pointerToMe)
{
    ContainerPlacer place = getPlacer(0, 0);

    mForceQuit = new RadioButton(_("Quit"), "quitdialog");
    mLogoutQuit = new RadioButton(_("Quit"), "quitdialog");
    mSwitchAccountServer = new RadioButton(_("Switch server"), "quitdialog");
    mSwitchCharacter = new RadioButton(_("Switch character"), "quitdialog");
    mOkButton = new Button(_("OK"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    //All states, when we're not logged in to someone.
    if (state == STATE_CHOOSE_SERVER ||
        state == STATE_CONNECT_SERVER ||
        state == STATE_LOGIN ||
        state == STATE_LOGIN_ATTEMPT ||
        state == STATE_UPDATE ||
        state == STATE_LOAD_DATA)
    {
        place(0, 0, mForceQuit, 3);
        mForceQuit->setSelected(true);
    }
    else
    {
        // Only added if we are connected to an accountserver or gameserver
        place(0, 0, mLogoutQuit, 3);
        place(0, 1, mSwitchAccountServer, 3);
        mLogoutQuit->setSelected(true);

        // Only added if we are connected to a gameserver
        if (state == STATE_GAME) place(0, 2, mSwitchCharacter, 3);
    }

    place(1, 3, mOkButton);
    place(2, 3, mCancelButton);

    reflowLayout(150, 0);
    setLocationRelativeTo(getParent());
    setVisible(true);
    mOkButton->requestFocus();
}

QuitDialog::~QuitDialog()
{
    if (mMyPointer) *mMyPointer = NULL;
    // Optional widgets, so delete them by hand.
    delete mForceQuit;
    delete mLogoutQuit;
    delete mSwitchAccountServer;
    delete mSwitchCharacter;
}

void QuitDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        if (mForceQuit->isSelected())
        {
            state = STATE_FORCE_QUIT;
        }
        else if (mLogoutQuit->isSelected())
        {
            state = STATE_EXIT;
        }
        else if (mSwitchAccountServer->isSelected())
        {
            state = STATE_SWITCH_SERVER;
        }
        else if (mSwitchCharacter->isSelected())
        {
            assert(state == STATE_GAME);

            Net::getCharHandler()->switchCharacter();
        }
    }
    scheduleDelete();
}
