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

#include "gui/widgets/button.h"
#include "gui/widgets/radiobutton.h"

#include "utils/gettext.h"

QuitDialog::QuitDialog(bool* quitGame, QuitDialog** pointerToMe):
    Window(_("Quit"), true, NULL), mQuitGame(quitGame), mMyPointer(pointerToMe)
{

    mLogoutQuit = new RadioButton(_("Quit"), "quitdialog");
    mForceQuit = new RadioButton(_("Quit"), "quitdialog");
    mSwitchAccountServer = new RadioButton(_("Switch server"), "quitdialog");
    mSwitchCharacter = new RadioButton(_("Switch character"), "quitdialog");
    mOkButton = new Button(_("OK"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    setContentSize(200, 91);

    mLogoutQuit->setPosition(5, 5);
    mForceQuit->setPosition(5, 5);
    mSwitchAccountServer->setPosition(5, 14 + mLogoutQuit->getHeight());
    mSwitchCharacter->setPosition(5,
           23 + mLogoutQuit->getHeight() + mSwitchAccountServer->getHeight());
    mCancelButton->setPosition(
           200 - mCancelButton->getWidth() - 5,
           91 - mCancelButton->getHeight() - 5);
    mOkButton->setPosition(
           mCancelButton->getX() - mOkButton->getWidth() - 5,
           91 - mOkButton->getHeight() - 5);

    //All states, when we're not logged in to someone.
    if (state == STATE_CHOOSE_SERVER ||
        state == STATE_CONNECT_ACCOUNT ||
        state == STATE_LOGIN ||
        state == STATE_LOGIN_ATTEMPT ||
        state == STATE_UPDATE)
    {
        mForceQuit->setSelected(true);
        add(mForceQuit);
    }
    else
    {
        // Only added if we are connected to an accountserver or gameserver
        mLogoutQuit->setSelected(true);
        add(mLogoutQuit);
        add(mSwitchAccountServer);

        // Only added if we are connected to a gameserver
        if (state == STATE_GAME) add(mSwitchCharacter);
    }

    add(mOkButton);
    add(mCancelButton);

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
            if ((state == STATE_GAME) && (mQuitGame))
            {
                *mQuitGame = true;
            }
            state = STATE_EXIT;
        }
        else if (mSwitchAccountServer->isSelected())
        {
            if ((state == STATE_GAME) && (mQuitGame))
            {
                *mQuitGame = true;
            }
            state = STATE_SWITCH_ACCOUNTSERVER_ATTEMPT;
        }
        else if (mSwitchCharacter->isSelected())
        {
            if (mQuitGame) *mQuitGame = true;

            state = STATE_SWITCH_CHARACTER;
        }

    }
    scheduleDelete();
}
