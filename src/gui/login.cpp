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

#include "login.h"

#include <string>

#include <guichan/widgets/label.hpp>

#include "../main.h"
#include "../logindata.h"

#include "button.h"
#include "checkbox.h"
#include "ok_dialog.h"
#include "passwordfield.h"
#include "textfield.h"

#include "../utils/gettext.h"

LoginDialog::LoginDialog(LoginData *loginData):
    Window(_("Login")), mLoginData(loginData)
{
    gcn::Label *userLabel = new gcn::Label(_("Name:"));
    gcn::Label *passLabel = new gcn::Label(_("Password:"));
    gcn::Label *serverLabel = new gcn::Label(_("Server:"));
    mUserField = new TextField(mLoginData->username);
    mPassField = new PasswordField(mLoginData->password);
    mServerField = new TextField(mLoginData->hostname);
    mKeepCheck = new CheckBox(_("Remember Username"), mLoginData->remember);
    mOkButton = new Button(_("Ok"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);
    mRegisterButton = new Button(_("Register"), "register", this);

    const int width = 250;
    const int height = 100;

    setContentSize(width, height);

    userLabel->setPosition(5, 5);
    passLabel->setPosition(5, 14 + userLabel->getHeight());
    serverLabel->setPosition(
            5, 23 + userLabel->getHeight() + passLabel->getHeight());
    mUserField->setPosition(65, 5);
    mPassField->setPosition(65, 14 + userLabel->getHeight());
    mServerField->setPosition(
            65, 23 + userLabel->getHeight() + passLabel->getHeight());
    mUserField->setWidth(width - 70);
    mPassField->setWidth(width - 70);
    mServerField->setWidth(width - 70);
    mKeepCheck->setPosition(4, 77);
    mCancelButton->setPosition(
            width - mCancelButton->getWidth() - 5,
            height - mCancelButton->getHeight() - 5);
    mOkButton->setPosition(
            mCancelButton->getX() - mOkButton->getWidth() - 5,
            height - mOkButton->getHeight() - 5);
    mRegisterButton->setPosition(
            mKeepCheck->getX() + mKeepCheck->getWidth() + 10,
            height - mRegisterButton->getHeight() - 5);

    mUserField->setActionEventId("ok");
    mPassField->setActionEventId("ok");
    mServerField->setActionEventId("ok");

    mUserField->addKeyListener(this);
    mPassField->addKeyListener(this);
    mServerField->addKeyListener(this);
    mUserField->addActionListener(this);
    mPassField->addActionListener(this);
    mServerField->addActionListener(this);
    mKeepCheck->addActionListener(this);

    add(userLabel);
    add(passLabel);
    add(serverLabel);
    add(mUserField);
    add(mPassField);
    add(mServerField);
    add(mKeepCheck);
    add(mOkButton);
    add(mCancelButton);
    add(mRegisterButton);

    setLocationRelativeTo(getParent());
    setVisible(true);

    if (mUserField->getText().empty()) {
        mUserField->requestFocus();
    } else {
        mPassField->requestFocus();
    }

    mOkButton->setEnabled(canSubmit());
}

LoginDialog::~LoginDialog()
{
}

void
LoginDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok" && canSubmit())
    {
        mLoginData->hostname = mServerField->getText();
        mLoginData->username = mUserField->getText();
        mLoginData->password = mPassField->getText();
        mLoginData->remember = mKeepCheck->isSelected();
        mLoginData->registerLogin = false;

        mOkButton->setEnabled(false);
        mRegisterButton->setEnabled(false);

        state = ACCOUNT_STATE;
    }
    else if (event.getId() == "cancel")
    {
        state = EXIT_STATE;
    }
    else if (event.getId() == "register")
    {
        // Transfer these fields on to the register dialog
        mLoginData->hostname = mServerField->getText();
        mLoginData->username = mUserField->getText();
        mLoginData->password = mPassField->getText();

        state = REGISTER_STATE;
    }
}

void
LoginDialog::keyPressed(gcn::KeyEvent &keyEvent)
{
    mOkButton->setEnabled(canSubmit());
}

bool
LoginDialog::canSubmit()
{
    return !mUserField->getText().empty() &&
           !mPassField->getText().empty() &&
           !mServerField->getText().empty() &&
           state == LOGIN_STATE;
}
