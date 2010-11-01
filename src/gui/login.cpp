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

#include "gui/login.h"

#include "client.h"
#include "configuration.h"

#include "gui/okdialog.h"
#include "gui/sdlinput.h"

#include "gui/widgets/button.h"
#include "gui/widgets/checkbox.h"
#include "gui/widgets/dropdown.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/passwordfield.h"
#include "gui/widgets/textfield.h"

#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/net.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

static const int MAX_SERVER_LIST_SIZE = 5;
static const int LOGIN_DIALOG_WIDTH = 220;
static const int LOGIN_DIALOG_HEIGHT = 140;
static const int FIELD_WIDTH = LOGIN_DIALOG_WIDTH - 70;

LoginDialog::LoginDialog(LoginData *loginData):
    Window(_("Login")),
    mLoginData(loginData)
{
    gcn::Label *userLabel = new Label(_("Name:"));
    gcn::Label *passLabel = new Label(_("Password:"));

    mUserField = new TextField(mLoginData->username);
    mPassField = new PasswordField(mLoginData->password);

    mKeepCheck = new CheckBox(_("Remember username"), mLoginData->remember);
    mRegisterButton = new Button(_("Register"), "register", this);
    mServerButton = new Button(_("Change Server"), "server", this);
    mLoginButton = new Button(_("Login"), "login", this);

    mUserField->setActionEventId("login");
    mPassField->setActionEventId("login");

    mUserField->addKeyListener(this);
    mPassField->addKeyListener(this);
    mUserField->addActionListener(this);
    mPassField->addActionListener(this);

    place(0, 0, userLabel);
    place(0, 1, passLabel);
    place(1, 0, mUserField, 3).setPadding(1);
    place(1, 1, mPassField, 3).setPadding(1);
    place(0, 5, mKeepCheck, 4);
    place(0, 6, mRegisterButton).setHAlign(LayoutCell::LEFT);
    place(2, 6, mServerButton);
    place(3, 6, mLoginButton);
    reflowLayout();

    addKeyListener(this);

    center();
    setVisible(true);

    if (mUserField->getText().empty())
        mUserField->requestFocus();
    else
        mPassField->requestFocus();

    mLoginButton->setEnabled(canSubmit());
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "login" && canSubmit())
    {
        mLoginData->username = mUserField->getText();
        mLoginData->password = mPassField->getText();
        mLoginData->remember = mKeepCheck->isSelected();
        mLoginData->registerLogin = false;

        mRegisterButton->setEnabled(false);
        mServerButton->setEnabled(false);
        mLoginButton->setEnabled(false);

        Client::setState(STATE_LOGIN_ATTEMPT);
    }
    else if (event.getId() == "server")
    {
        Client::setState(STATE_SWITCH_SERVER);
    }
    else if (event.getId() == "register")
    {
        if (Net::getLoginHandler()->isRegistrationEnabled())
        {
            mLoginData->username = mUserField->getText();
            mLoginData->password = mPassField->getText();
            Client::setState(STATE_REGISTER_PREP);
        }
        else
        {
            new OkDialog(_("Registration disabled"), _("You need to use the "
                           "website to register an account for this server."));
        }
    }
}

void LoginDialog::keyPressed(gcn::KeyEvent &keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ESCAPE)
    {
        action(gcn::ActionEvent(NULL, mServerButton->getActionEventId()));
    }
    else if (key.getValue() == Key::ENTER)
    {
        action(gcn::ActionEvent(NULL, mLoginButton->getActionEventId()));
    }
    else
    {
        mLoginButton->setEnabled(canSubmit());
    }
}

bool LoginDialog::canSubmit() const
{
    return !mUserField->getText().empty() &&
           !mPassField->getText().empty() &&
           Client::getState() == STATE_LOGIN;
}
