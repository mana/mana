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

#include "gui/quitdialog.h"

#include "client.h"

#include "gui/sdlinput.h"

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
    mForceQuit = new RadioButton(_("Quit"), "quitdialog");
    mLogoutQuit = new RadioButton(_("Quit"), "quitdialog");
    mSwitchAccountServer = new RadioButton(_("Switch server"), "quitdialog");
    mSwitchCharacter = new RadioButton(_("Switch character"), "quitdialog");
    mOkButton = new Button(_("OK"), "ok", this);
    mOkButton->setButtonIcon("button-icon-confirm.png");
    mCancelButton = new Button(_("Cancel"), "cancel", this);
    mCancelButton->setButtonIcon("button-icon-cancel.png");

    addKeyListener(this);

    ContainerPlacer place = getPlacer(0, 0);

    const State state = Client::getState();

    // All states, when we're not logged in to someone.
    if (state == STATE_CHOOSE_SERVER ||
        state == STATE_CONNECT_SERVER ||
        state == STATE_LOGIN ||
        state == STATE_LOGIN_ATTEMPT ||
        state == STATE_UPDATE ||
        state == STATE_LOAD_DATA)
    {
        placeOption(place, mForceQuit);
    }
    else
    {
        // Only added if we are connected to an accountserver or gameserver
        placeOption(place, mLogoutQuit);
        placeOption(place, mSwitchAccountServer);

        // Only added if we are connected to a gameserver
        if (state == STATE_GAME) placeOption(place, mSwitchCharacter);
    }

    mOptions[0]->setSelected(true);

    place = getPlacer(0, 1);

    place(1, 0, mOkButton);
    place(2, 0, mCancelButton);

    reflowLayout(150, 0);
    setLocationRelativeTo(getParent());
    setVisible(true);
    requestModalFocus();
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

void QuitDialog::placeOption(ContainerPlacer &place, gcn::RadioButton *option)
{
    place(0, mOptions.size(), option, 3);
    mOptions.push_back(option);
}

void QuitDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        if (mForceQuit->isSelected())
        {
            Client::setState(STATE_FORCE_QUIT);
        }
        else if (mLogoutQuit->isSelected())
        {
            Client::setState(STATE_EXIT);
        }
        else if (mSwitchAccountServer->isSelected())
        {
            Client::setState(STATE_SWITCH_SERVER);
        }
        else if (mSwitchCharacter->isSelected())
        {
            assert(Client::getState() == STATE_GAME);

            Net::getCharHandler()->switchCharacter();
        }
    }
    scheduleDelete();
}

void QuitDialog::keyPressed(gcn::KeyEvent &keyEvent)
{
    const gcn::Key &key = keyEvent.getKey();
    int dir = 0;

    switch (key.getValue())
    {
        case Key::ENTER:
            action(gcn::ActionEvent(NULL, mOkButton->getActionEventId()));
            break;
        case Key::ESCAPE:
            action(gcn::ActionEvent(NULL, mCancelButton->getActionEventId()));
            break;
        case Key::UP:
            dir = -1;
            break;
        case Key::DOWN:
            dir = 1;
            break;
    }

    if (dir != 0)
    {
        std::vector<gcn::RadioButton*>::iterator it = mOptions.begin();

        for (; it < mOptions.end(); it++)
        {
            if ((*it)->isSelected())
                break;
        }

        if (it == mOptions.end())
        {
            mOptions[0]->setSelected(true);
            return;
        }
        else if (it == mOptions.begin() && dir < 0)
            it = mOptions.end();

        it += dir;

        if (it == mOptions.end())
            it = mOptions.begin();

        (*it)->setSelected(true);
    }
}
