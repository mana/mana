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

#include "gui/register.h"

#include "configuration.h"
#include "log.h"
#include "main.h"

#include "gui/login.h"
#include "gui/okdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/passwordfield.h"
#include "gui/widgets/radiobutton.h"
#include "gui/widgets/textfield.h"

#include "net/logindata.h"

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
    mWrongDataNoticeListener(new WrongDataNoticeListener),
    mLoginData(loginData)
{
    gcn::Label *userLabel = new Label(_("Name:"));
    gcn::Label *passwordLabel = new Label(_("Password:"));
    gcn::Label *confirmLabel = new Label(_("Confirm:"));
#ifdef EATHENA_SUPPORT
    gcn::Label *serverLabel = new Label(_("Server:"));
    gcn::Label *portLabel = new Label(_("Port:"));
#else
    gcn::Label *emailLabel = new Label(_("Email:"));
#endif
    mUserField = new TextField(loginData->username);
    mPasswordField = new PasswordField(loginData->password);
    mConfirmField = new PasswordField;
#ifdef EATHENA_SUPPORT
    mServerField = new TextField(loginData->hostname);
    mPortField = new TextField(toString(loginData->port));
    mMaleButton = new RadioButton(_("Male"), "sex", true);
    mFemaleButton = new RadioButton(_("Female"), "sex", false);
#else
    mEmailField = new TextField;
#endif
    mRegisterButton = new Button(_("Register"), "register", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    ContainerPlacer place;
    place = getPlacer(0, 0);
    place(0, 0, userLabel);
    place(0, 1, passwordLabel);
    place(0, 2, confirmLabel);
#ifdef EATHENA_SUPPORT
    place(1, 3, mMaleButton);
    place(2, 3, mFemaleButton);
    place(0, 4, serverLabel);
    place(0, 5, portLabel);
#else
    place(0, 3, emailLabel);
#endif
    place(1, 0, mUserField, 3).setPadding(2);
    place(1, 1, mPasswordField, 3).setPadding(2);
    place(1, 2, mConfirmField, 3).setPadding(2);
#ifdef EATHENA_SUPPORT
    place(1, 4, mServerField, 3).setPadding(2);
    place(1, 5, mPortField, 3).setPadding(2);
#else
    place(1, 3, mEmailField, 3).setPadding(2);
#endif
    place = getPlacer(0, 2);
    place(1, 0, mRegisterButton);
    place(2, 0, mCancelButton);
    reflowLayout(250, 0);

    mUserField->addKeyListener(this);
    mPasswordField->addKeyListener(this);
    mConfirmField->addKeyListener(this);
#ifdef EATHENA_SUPPORT
    mServerField->addKeyListener(this);
    mPortField->addKeyListener(this);
#endif

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

#ifdef EATHENA_SUPPORT
    mServerField->setActionEventId("register");
    mPortField->setActionEventId("register");

    mServerField->addActionListener(this);
    mPortField->addActionListener(this);
#endif

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
        state = STATE_LOGIN;
    }
    else if (event.getId() == "register" && canSubmit())
    {
        const std::string user = mUserField->getText();
        logger->log("RegisterDialog::register Username is %s", user.c_str());

        std::string errorMsg;
        int error = 0;

        if (user.length() < LEN_MIN_USERNAME)
        {
            // Name too short
            errorMsg = strprintf
                (_("The username needs to be at least %d characters long."),
                 LEN_MIN_USERNAME);
            error = 1;
        }
        else if (user.length() > LEN_MAX_USERNAME - 1 )
        {
            // Name too long
            errorMsg = strprintf
                (_("The username needs to be less than %d characters long."),
                 LEN_MAX_USERNAME);
            error = 1;
        }
        else if (mPasswordField->getText().length() < LEN_MIN_PASSWORD)
        {
            // Pass too short
            errorMsg = strprintf
                (_("The password needs to be at least %d characters long."),
                 LEN_MIN_PASSWORD);
            error = 2;
        }
        else if (mPasswordField->getText().length() > LEN_MAX_PASSWORD - 1 )
        {
            // Pass too long
            errorMsg = strprintf
                (_("The password needs to be less than %d characters long."),
                 LEN_MAX_PASSWORD);
            error = 2;
        }
        else if (mPasswordField->getText() != mConfirmField->getText())
        {
            // Password does not match with the confirmation one
            errorMsg = _("Passwords do not match.");
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
                mPasswordField->setText("");
                mConfirmField->setText("");

                mWrongDataNoticeListener->setTarget(this->mPasswordField);
            }

            OkDialog *dlg = new OkDialog(_("Error"), errorMsg);
            dlg->addActionListener(mWrongDataNoticeListener);
        }
        else
        {
            // No errors detected, register the new user.
            mRegisterButton->setEnabled(false);

            mLoginData->username = mUserField->getText();
            mLoginData->password = mPasswordField->getText();
#ifdef EATHENA_SUPPORT
            mLoginData->hostname = mServerField->getText();
            mLoginData->port = getUShort(mPortField->getText());
            mLoginData->sex =
                    mFemaleButton->isSelected() ? GENDER_FEMALE : GENDER_MALE;
#else
            mLoginData->email = mEmailField->getText();
#endif
            mLoginData->registerLogin = true;

#ifdef TMWSERV_SUPPORT
            state = STATE_REGISTER_ATTEMPT;
#else
            state = STATE_ACCOUNT;
#endif
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
#ifdef EATHENA_SUPPORT
           !mServerField->getText().empty() &&
           isUShort(mPortField->getText()) &&
#endif
           state == STATE_REGISTER;
}

#ifdef EATHENA_SUPPORT
bool RegisterDialog::isUShort(const std::string &str)
{
    if (str.empty())
    {
        return false;
    }
    unsigned long l = 0;
    for (std::string::const_iterator strPtr = str.begin(), strEnd = str.end();
         strPtr != strEnd; ++strPtr)
    {
        if (*strPtr < '0' || *strPtr > '9')
        {
            return false;
        }
        l = l * 10 + (*strPtr - '0'); // *strPtr - '0' will never be negative
        if (l > 65535)
        {
            return false;
        }
    }
    return true;
}

unsigned short RegisterDialog::getUShort(const std::string &str)
{
    unsigned long l = 0;
    for (std::string::const_iterator strPtr = str.begin(), strEnd = str.end();
         strPtr != strEnd; ++strPtr)
    {
        l = l * 10 + (*strPtr - '0');
    }
    return static_cast<unsigned short>(l);
}
#endif
