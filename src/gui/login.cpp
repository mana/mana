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

void
WrongDataNoticeListener::setTarget(gcn::TextField *textField)
{
    mTarget = textField;
}

void
WrongDataNoticeListener::action(const std::string &eventId, gcn::Widget *widget)
{
    if (eventId == "ok")
    {
        // Reset the field
        mTarget->setText("");
        mTarget->setCaretPosition(0);
        mTarget->requestFocus();
    }
}

LoginDialog::LoginDialog(LoginData *loginData):
    Window("Login"), mLoginData(loginData)
{
    gcn::Label *userLabel = new gcn::Label("Name:");
    gcn::Label *passLabel = new gcn::Label("Password:");
    mUserField = new TextField(mLoginData->username);
    mPassField = new PasswordField(mLoginData->password);
    mKeepCheck = new CheckBox("Keep", mLoginData->remember);
    mOkButton = new Button("OK", "ok", this);
    mCancelButton = new Button("Cancel", "cancel", this);
    mRegisterButton = new Button("Register", "register", this);

    setContentSize(200, 91);

    userLabel->setPosition(5, 5);
    passLabel->setPosition(5, 14 + userLabel->getHeight());
    mUserField->setPosition(65, 5);
    mPassField->setPosition(65, 14 + userLabel->getHeight());
    mUserField->setWidth(130);
    mPassField->setWidth(130);
    mKeepCheck->setPosition(4, 68);
    mCancelButton->setPosition(
            200 - mCancelButton->getWidth() - 5,
            91 - mCancelButton->getHeight() - 5);
    mOkButton->setPosition(
            mCancelButton->getX() - mOkButton->getWidth() - 5,
            91 - mOkButton->getHeight() - 5);
    mRegisterButton->setPosition(
            mKeepCheck->getX() + mKeepCheck->getWidth() + 10,
            91 - mRegisterButton->getHeight() - 5);

    mUserField->setEventId("ok");
    mPassField->setEventId("ok");

    mUserField->addActionListener(this);
    mPassField->addActionListener(this);
    mKeepCheck->addActionListener(this);

    add(userLabel);
    add(passLabel);
    add(mUserField);
    add(mPassField);
    add(mKeepCheck);
    add(mOkButton);
    add(mCancelButton);
    add(mRegisterButton);

    setLocationRelativeTo(getParent());

    if (mUserField->getText().empty()) {
        mUserField->requestFocus();
    } else {
        mPassField->requestFocus();
    }

    mWrongDataNoticeListener = new WrongDataNoticeListener();
}

LoginDialog::~LoginDialog()
{
    delete mWrongDataNoticeListener;
}

void
LoginDialog::action(const std::string &eventId, gcn::Widget *widget)
{
    if (eventId == "ok")
    {
        // Check login
        if (mUserField->getText().empty())
        {
            mWrongDataNoticeListener->setTarget(mPassField);
            OkDialog *dlg = new OkDialog("Error", "Enter your username first");
            dlg->addActionListener(mWrongDataNoticeListener);
        }
        else
        {
            mLoginData->username = mUserField->getText();
            mLoginData->password = mPassField->getText();
            mLoginData->remember = mKeepCheck->isMarked();

            mOkButton->setEnabled(false);
            mRegisterButton->setEnabled(false);

            state = STATE_LOGIN_ATTEMPT;
        }
    }
    else if (eventId == "cancel")
    {
        state = STATE_EXIT;
    }
    else if (eventId == "register")
    {
        state = STATE_REGISTER;
    }
}
