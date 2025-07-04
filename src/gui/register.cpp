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

#include "gui/register.h"

#include "client.h"
#include "log.h"

#include "gui/logindialog.h"
#include "gui/okdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/passwordfield.h"
#include "gui/widgets/radiobutton.h"
#include "gui/widgets/textfield.h"

#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/net.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

void WrongDataNoticeListener::setTarget(gcn::TextField *textField)
{
    mTarget = textField;
}

void WrongDataNoticeListener::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
        mTarget->requestFocus();
}

RegisterDialog::RegisterDialog(LoginData *loginData):
    Window(_("Register")),
    mEmailField(nullptr),
    mMaleButton(nullptr),
    mFemaleButton(nullptr),
    mWrongDataNoticeListener(new WrongDataNoticeListener),
    mLoginData(loginData)
{
    int optionalActions = Net::getLoginHandler()->supportedOptionalActions();

    gcn::Label *userLabel = new Label(_("Name:"));
    gcn::Label *passwordLabel = new Label(_("Password:"));
    gcn::Label *confirmLabel = new Label(_("Confirm:"));
    mUserField = new TextField(loginData->username);
    mPasswordField = new PasswordField(loginData->password);
    mConfirmField = new PasswordField;
    mRegisterButton = new Button(_("Register"), "register", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    ContainerPlacer place;
    place = getPlacer(0, 0);
    place(0, 0, userLabel);
    place(0, 1, passwordLabel);
    place(0, 2, confirmLabel);

    place(1, 0, mUserField, 3).setPadding(2);
    place(1, 1, mPasswordField, 3).setPadding(2);
    place(1, 2, mConfirmField, 3).setPadding(2);

    int row = 3;

    if (optionalActions & Net::LoginHandler::SetGenderOnRegister)
    {
        mMaleButton = new RadioButton(_("Male"), "sex", true);
        mFemaleButton = new RadioButton(_("Female"), "sex", false);
        place(1, row, mMaleButton);
        place(2, row, mFemaleButton);

        row++;
    }

    if (optionalActions & Net::LoginHandler::SetEmailOnRegister)
    {
        gcn::Label *emailLabel = new Label(_("Email:"));
        mEmailField = new TextField;
        place(0, row, emailLabel);
        place(1, row, mEmailField, 3).setPadding(2);

        row++;
    }

    place = getPlacer(0, 2);
    place(1, 0, mRegisterButton);
    place(2, 0, mCancelButton);
    reflowLayout(250, 0);

    mUserField->addKeyListener(this);
    mPasswordField->addKeyListener(this);
    mConfirmField->addKeyListener(this);

    /* TODO:
     * This is a quick and dirty way to respond to the ENTER key, regardless of
     * which text field is selected. There may be a better way now with the new
     * input system of Guichan 0.6.0. See also the login dialog.
     */
    mUserField->setActionEventId("register");
    mPasswordField->setActionEventId("register");
    mConfirmField->setActionEventId("register");

    mUserField->addActionListener(this);
    mPasswordField->addActionListener(this);
    mConfirmField->addActionListener(this);

    center();
    setVisible(true);
    mUserField->requestFocus();
    mUserField->setCaretPosition(mUserField->getText().length());

    mRegisterButton->setEnabled(canSubmit());
}

RegisterDialog::~RegisterDialog()
{
    delete mWrongDataNoticeListener;
}

void RegisterDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "cancel")
    {
        Client::setState(STATE_LOGIN);
    }
    else if (event.getId() == "register" && canSubmit())
    {
        const std::string user = mUserField->getText();
        Log::info("RegisterDialog::register Username is %s", user.c_str());

        std::string errorMessage;
        int error = 0;

        unsigned int minUser = Net::getLoginHandler()->getMinUserNameLength();
        unsigned int maxUser = Net::getLoginHandler()->getMaxUserNameLength();
        unsigned int minPass = Net::getLoginHandler()->getMinPasswordLength();
        unsigned int maxPass = Net::getLoginHandler()->getMaxPasswordLength();

        if (user.length() < minUser)
        {
            // Name too short
            errorMessage = strprintf
                (_("The username needs to be at least %d characters long."),
                 minUser);
            error = 1;
        }
        else if (user.length() > maxUser - 1)
        {
            // Name too long
            errorMessage = strprintf
                (_("The username needs to be less than %d characters long."),
                 maxUser);
            error = 1;
        }
        else if (mPasswordField->getText().length() < minPass)
        {
            // Pass too short
            errorMessage = strprintf
                (_("The password needs to be at least %d characters long."),
                 minPass);
            error = 2;
        }
        else if (mPasswordField->getText().length() > maxPass - 1)
        {
            // Pass too long
            errorMessage = strprintf
                (_("The password needs to be less than %d characters long."),
                 maxPass);
            error = 2;
        }
        else if (mPasswordField->getText() != mConfirmField->getText())
        {
            // Password does not match with the confirmation one
            errorMessage = _("Passwords do not match.");
            error = 2;
        }

        // TODO: Check if a valid email address was given

        if (error > 0)
        {
            if (error == 1)
            {
                mWrongDataNoticeListener->setTarget(this->mUserField);
            }
            else if (error == 2)
            {
                // Reset password confirmation
                mPasswordField->setText(std::string());
                mConfirmField->setText(std::string());

                mWrongDataNoticeListener->setTarget(this->mPasswordField);
            }

            auto *dlg = new OkDialog(_("Error"), errorMessage);
            dlg->addActionListener(mWrongDataNoticeListener);
        }
        else
        {
            // No errors detected, register the new user.
            mRegisterButton->setEnabled(false);

            mLoginData->username = mUserField->getText();
            mLoginData->password = mPasswordField->getText();
            if (mFemaleButton)
                mLoginData->gender = mFemaleButton->isSelected() ?
                                     Gender::Female : Gender::Male;
            if (mEmailField)
                mLoginData->email = mEmailField->getText();
            mLoginData->registerLogin = true;

            Client::setState(STATE_REGISTER_ATTEMPT);
        }
    }
}

void RegisterDialog::keyPressed(gcn::KeyEvent &keyEvent)
{
    mRegisterButton->setEnabled(canSubmit());
}

bool RegisterDialog::canSubmit() const
{
    return !mUserField->getText().empty() &&
           !mPasswordField->getText().empty() &&
           !mConfirmField->getText().empty() &&
           Client::getState() == STATE_REGISTER;
}
