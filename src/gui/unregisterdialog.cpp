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

#include "unregisterdialog.h"

#include <string>
#include <sstream>

#include <guichan/widgets/label.hpp>

#include "../main.h"
#include "../log.h"
#include "../logindata.h"

#include "button.h"
#include "checkbox.h"
#include "login.h"
#include "passwordfield.h"
#include "textfield.h"
#include "ok_dialog.h"

UnRegisterDialog::UnRegisterDialog(Window *parent, LoginData *loginData):
    Window("Unregister", true, parent),
    mWrongDataNoticeListener(new WrongDataNoticeListener()),
    mLoginData(loginData)
{
    gcn::Label *userLabel = new gcn::Label("Name:");
    gcn::Label *passwordLabel = new gcn::Label("Password:");
    mUserField = new TextField(mLoginData->username);
    mPasswordField = new PasswordField(mLoginData->password);
    mUnRegisterButton = new Button("Unregister", "unregister", this);
    mCancelButton = new Button("Cancel", "cancel", this);

    const int width = 200;
    const int height = 70;
    setContentSize(width, height);

    mUserField->setPosition(65, 5);
    mUserField->setWidth(130);
    mPasswordField->setPosition(
            65, mUserField->getY() + mUserField->getHeight() + 7);
    mPasswordField->setWidth(130);

    userLabel->setPosition(5, mUserField->getY() + 1);
    passwordLabel->setPosition(5, mPasswordField->getY() + 1);

    mCancelButton->setPosition(
            width - 5 - mCancelButton->getWidth(),
            height - 5 - mCancelButton->getHeight());
    mUnRegisterButton->setPosition(
            mCancelButton->getX() - 5 - mUnRegisterButton->getWidth(),
            mCancelButton->getY());

    add(userLabel);
    add(passwordLabel);
    add(mUserField);
    add(mPasswordField);
    add(mUnRegisterButton);
    add(mCancelButton);

    setLocationRelativeTo(getParent());
    setVisible(true);
    mPasswordField->requestFocus();
}

UnRegisterDialog::~UnRegisterDialog()
{
    delete mWrongDataNoticeListener;
}

void
UnRegisterDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "cancel")
    {
        scheduleDelete();
    }
    else if (event.getId() == "unregister")
    {
        const std::string username = mUserField->getText();
        const std::string password = mPasswordField->getText();
        logger->log("UnregisterDialog::unregistered, Username is %s",
                     username.c_str());

        std::stringstream errorMsg;
        int error = 0;

        // Check login
        if (username.empty())
        {
            // No username
            errorMsg << "Enter your username first.";
            error = 1;
        }
        else if (username.length() < LEN_MIN_USERNAME)
        {
            // Name too short
            errorMsg << "The username needs to be at least "
                     << LEN_MIN_USERNAME
                     << " characters long.";
            error = 1;
        }
        else if (username.length() > LEN_MAX_USERNAME - 1 )
        {
            // Name too long
            errorMsg << "The username needs to be less than "
                     << LEN_MAX_USERNAME
                     << " characters long.";
            error = 1;
        }
        else if (password.length() < LEN_MIN_PASSWORD)
        {
            // Pass too short
            errorMsg << "The password needs to be at least "
                     << LEN_MIN_PASSWORD
                     << " characters long.";
            error = 2;
        }
        else if (password.length() > LEN_MAX_PASSWORD - 1 )
        {
            // Pass too long
            errorMsg << "The password needs to be less than "
                     << LEN_MAX_PASSWORD
                     << " characters long.";
            error = 2;
        }

        if (error > 0)
        {
            if (error == 1)
            {
                mWrongDataNoticeListener->setTarget(this->mUserField);
            }
            else if (error == 2)
            {
                mWrongDataNoticeListener->setTarget(this->mPasswordField);
            }

            OkDialog *dlg = new OkDialog("Error", errorMsg.str());
            dlg->addActionListener(mWrongDataNoticeListener);
        }
        else
        {
            // No errors detected, unregister the new user.
            mUnRegisterButton->setEnabled(false);
            mLoginData->username = username;
            mLoginData->password = password;
            state = STATE_UNREGISTER_ATTEMPT;
            scheduleDelete();
        }
    }
}
