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
 *
 *  $Id$
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

#include "widgets/layout.h"

#include "../utils/gettext.h"

LoginDialog::LoginDialog(LoginData *loginData):
    Window(_("Login")), mLoginData(loginData)
{
    gcn::Label *userLabel = new gcn::Label(_("Name:"));
    gcn::Label *passLabel = new gcn::Label(_("Password:"));
    mUserField = new TextField(mLoginData->username);
    mPassField = new PasswordField(mLoginData->password);
    mKeepCheck = new CheckBox(_("Keep"), mLoginData->remember);
    mOkButton = new Button(_("Ok"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);
    mRegisterButton = new Button(_("Register"), "register", this);

    mUserField->setActionEventId("ok");
    mPassField->setActionEventId("ok");

    mUserField->addKeyListener(this);
    mPassField->addKeyListener(this);
    mUserField->addActionListener(this);
    mPassField->addActionListener(this);
    mKeepCheck->addActionListener(this);

    place(0, 0, userLabel);
    place(0, 1, passLabel);
    place(1, 0, mUserField, 3).setPadding(2);
    place(1, 1, mPassField, 3).setPadding(2);
    place(0, 2, mKeepCheck, 4);
    place(0, 3, mRegisterButton).setHAlign(Cell::LEFT);
    place(2, 3, mOkButton);
    place(3, 3, mCancelButton);
    getLayout().setColWidth(1, 20);
    reflowLayout();
    forgetLayout();

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
        mLoginData->username = mUserField->getText();
        mLoginData->password = mPassField->getText();
        mLoginData->remember = mKeepCheck->isMarked();
        mLoginData->registerLogin = false;

        mOkButton->setEnabled(false);
        mRegisterButton->setEnabled(false);

        state = STATE_LOGIN_ATTEMPT;
    }
    else if (event.getId() == "cancel")
    {
        state = STATE_FORCE_QUIT;
    }
    else if (event.getId() == "register")
    {
        // Transfer these fields on to the register dialog
        mLoginData->username = mUserField->getText();
        mLoginData->password = mPassField->getText();

        state = STATE_REGISTER;
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
           state == STATE_LOGIN;
}
