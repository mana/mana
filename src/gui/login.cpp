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
    this->target = textField;
}

void
WrongDataNoticeListener::action(const std::string &eventId)
{
    if (eventId == "ok")
    {
        // Reset the field
        target->setText("");
        target->setCaretPosition(0);
        target->requestFocus();
    }
}

LoginDialog::LoginDialog(LoginData *loginData):
    Window("Login"), mLoginData(loginData)
{
    userLabel = new gcn::Label("Name:");
    passLabel = new gcn::Label("Password:");
    serverLabel = new gcn::Label("Server:");
    userField = new TextField(mLoginData->username);
    passField = new PasswordField(mLoginData->password);
    serverField = new TextField(mLoginData->hostname);
    keepCheck = new CheckBox("Keep", false);
    okButton = new Button("OK", "ok", this);
    cancelButton = new Button("Cancel", "cancel", this);
    registerButton = new Button("Register", "register", this);

    setContentSize(200, 100);

    userLabel->setPosition(5, 5);
    passLabel->setPosition(5, 14 + userLabel->getHeight());
    serverLabel->setPosition(
            5, 23 + userLabel->getHeight() + passLabel->getHeight());
    userField->setPosition(65, 5);
    passField->setPosition(65, 14 + userLabel->getHeight());
    serverField->setPosition(
            65, 23 + userLabel->getHeight() + passLabel->getHeight());
    userField->setWidth(130);
    passField->setWidth(130);
    serverField->setWidth(130);
    keepCheck->setPosition(4, 77);
    keepCheck->setMarked(mLoginData->remember);
    cancelButton->setPosition(
            200 - cancelButton->getWidth() - 5,
            100 - cancelButton->getHeight() - 5);
    okButton->setPosition(
            cancelButton->getX() - okButton->getWidth() - 5,
            100 - okButton->getHeight() - 5);
    registerButton->setPosition(keepCheck->getX() + keepCheck->getWidth() + 10,
            100 - registerButton->getHeight() - 5);

    userField->setEventId("ok");
    passField->setEventId("ok");
    serverField->setEventId("ok");

    userField->addActionListener(this);
    passField->addActionListener(this);
    serverField->addActionListener(this);
    keepCheck->addActionListener(this);

    add(userLabel);
    add(passLabel);
    add(serverLabel);
    add(userField);
    add(passField);
    add(serverField);
    add(keepCheck);
    add(okButton);
    add(cancelButton);
    add(registerButton);

    setLocationRelativeTo(getParent());

    if (!userField->getText().length()) {
        userField->requestFocus();
    } else {
        passField->requestFocus();
    }

    wrongDataNoticeListener = new WrongDataNoticeListener();
}

LoginDialog::~LoginDialog()
{
    delete wrongDataNoticeListener;
}

void
LoginDialog::action(const std::string& eventId)
{
    if (eventId == "ok")
    {
        // Check login
        if (userField->getText().length() == 0)
        {
            wrongDataNoticeListener->setTarget(this->passField);
            OkDialog *dlg = new OkDialog("Error", "Enter your username first");
            dlg->addActionListener(wrongDataNoticeListener);
        }
        else
        {
            mLoginData->hostname = serverField->getText();
            mLoginData->username = userField->getText();
            mLoginData->password = passField->getText();
            mLoginData->remember = keepCheck->isMarked();

            okButton->setEnabled(false);
            //cancelButton->setEnabled(false);
            registerButton->setEnabled(false);

            state = ACCOUNT_STATE;
        }
    }
    else if (eventId == "cancel")
    {
        state = EXIT_STATE;
    }
    else if (eventId == "register")
    {
        state = REGISTER_STATE;
    }
}
