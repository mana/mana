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

#include <string>
#include <vector>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "checkbox.h"
#include "login.h"
#include "ok_dialog.h"
#include "passwordfield.h"
#include "textfield.h"

#include "widgets/layout.h"

#include "../main.h"
#include "../logindata.h"
#include "../configuration.h"

#include "../utils/gettext.h"
#include "../utils/tostring.h"

static const int MAX_SERVER_LIST_SIZE = 5;
static const int LOGIN_DIALOG_WIDTH = 220;
static const int LOGIN_DIALOG_HEIGHT = 140;
static const int FIELD_WIDTH = LOGIN_DIALOG_WIDTH - 70;

LoginDialog::LoginDialog(LoginData *loginData):
    Window(_("Login")), mLoginData(loginData)
{
    gcn::Label *userLabel = new gcn::Label(_("Name:"));
    gcn::Label *passLabel = new gcn::Label(_("Password:"));
    gcn::Label *serverLabel = new gcn::Label(_("Server:"));
    gcn::Label *portLabel = new gcn::Label(_("Port:"));
    gcn::Label *dropdownLabel = new gcn::Label(_("Recent:"));
    std::vector<std::string> dfltServer;
    dfltServer.push_back("www.aethyra.org"); 
    dfltServer.push_back("www.aethyra.org");
    dfltServer.push_back("209.168.213.109");
    std::vector<std::string> dfltPort;
    dfltPort.push_back("21001");
    dfltPort.push_back("22001");
    dfltPort.push_back("21001");
    mServerList = new DropDownList("MostRecent00", dfltServer, dfltPort,
                                   MAX_SERVER_LIST_SIZE);
    mServerListBox = new gcn::ListBox(mServerList);
    mServerScrollArea = new ScrollArea();

    mUserField = new TextField(mLoginData->username);
    mPassField = new PasswordField(mLoginData->password);
    mServerField = new TextField(mServerList->getServerAt(0));
    mPortField = new TextField(mServerList->getPortAt(0));
    mServerDropDown = new DropDown(mServerList,
                                   mServerScrollArea,
                                   mServerListBox);

    mKeepCheck = new CheckBox(_("Keep"), mLoginData->remember);
    mOkButton = new Button(_("OK"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);
    mRegisterButton = new Button(_("Register"), "register", this);

    mUserField->setActionEventId("ok");
    mPassField->setActionEventId("ok");
    mServerField->setActionEventId("ok");
    mServerDropDown->setActionEventId("changeSelection");

    mUserField->addKeyListener(this);
    mPassField->addKeyListener(this);
    mServerField->addKeyListener(this);
    mPortField->addKeyListener(this);
    mServerDropDown->addKeyListener(this);
    mUserField->addActionListener(this);
    mPassField->addActionListener(this);
    mServerField->addActionListener(this);
    mPortField->addActionListener(this);
    mServerDropDown->addActionListener(this);
    mKeepCheck->addActionListener(this);

    place(0, 0, userLabel);
    place(0, 1, passLabel);
    place(0, 2, serverLabel);
    place(0, 3, portLabel);
    place(0, 4, dropdownLabel);
    place(1, 0, mUserField, 3).setPadding(1);
    place(1, 1, mPassField, 3).setPadding(1);
    place(1, 2, mServerField, 3).setPadding(1);
    place(1, 3, mPortField, 3).setPadding(1);
    place(1, 4, mServerDropDown, 3).setPadding(1);
    place(0, 5, mKeepCheck, 4);
    place(0, 6, mRegisterButton).setHAlign(LayoutCell::LEFT);
    place(2, 6, mCancelButton);
    place(3, 6, mOkButton);
    reflowLayout(250, 0);

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
    delete mServerList;
    delete mServerListBox;
    delete mServerScrollArea;
}

void LoginDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok" && canSubmit())
    {
        mLoginData->hostname = mServerField->getText();
        mLoginData->port = getUShort(mPortField->getText());
        mLoginData->username = mUserField->getText();
        mLoginData->password = mPassField->getText();
        mLoginData->remember = mKeepCheck->isSelected();
        mLoginData->registerLogin = false;

        mOkButton->setEnabled(false);
        mRegisterButton->setEnabled(false);
        mServerList->save(mServerField->getText(), mPortField->getText());
        state = ACCOUNT_STATE;
    }
    else if (event.getId() == "changeSelection")
    {
        int selected = mServerListBox->getSelected();
        mServerField->setText(mServerList->getServerAt(selected));
        mPortField->setText(mServerList->getPortAt(selected));
    }
    else if (event.getId() == "cancel")
    {
        state = EXIT_STATE;
    }
    else if (event.getId() == "register")
    {
        // Transfer these fields on to the register dialog
        mLoginData->hostname = mServerField->getText();
        if (isUShort(mPortField->getText()))
        {
            mLoginData->port = getUShort(mPortField->getText());
        }
        else
        {
            mLoginData->port = 6901;
        }
        mLoginData->username = mUserField->getText();
        mLoginData->password = mPassField->getText();

        state = REGISTER_STATE;
    }
}

void LoginDialog::keyPressed(gcn::KeyEvent &keyEvent)
{
    mOkButton->setEnabled(canSubmit());
}

bool LoginDialog::canSubmit()
{
    return !mUserField->getText().empty() &&
           !mPassField->getText().empty() &&
           !mServerField->getText().empty() &&
           isUShort(mPortField->getText()) &&
           state == LOGIN_STATE;
}

bool LoginDialog::isUShort(const std::string &str)
{
    if (str == "")
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

unsigned short LoginDialog::getUShort(const std::string &str)
{
    unsigned long l = 0;
    for (std::string::const_iterator strPtr = str.begin(), strEnd = str.end();
         strPtr != strEnd; ++strPtr)
    {
        l = l * 10 + (*strPtr - '0');
    }
    return static_cast<unsigned short>(l);
}

/**
 * LoginDialog::DropDownList
 */

void LoginDialog::DropDownList::saveEntry(const std::string &server,
                                          const std::string &port, int &saved)
{
    if (saved < MAX_SERVER_LIST_SIZE && server != "")
    {
        config.setValue(mConfigPrefix + "Server" + toString(saved), server);
        config.setValue(mConfigPrefix + "Port" + toString(saved), port);
        ++saved;
    }
}

LoginDialog::DropDownList::DropDownList(std::string prefix,
                                        std::vector<std::string> dflt,
                                        std::vector<std::string> dfltPort,
                                        int maxEntries) :
                                        mConfigPrefix(prefix),
                                        mMaxEntries(maxEntries)
{
    for (int i = 0; i < maxEntries; ++i)
    {
        std::string server = config.getValue(mConfigPrefix + "Server" +
                                             toString(i), "");
        if (server == "") // Just in case had original config entries
        {
            server = config.getValue(mConfigPrefix + "ServerList" +
                                     toString(i), "");
        }
        std::string port = config.getValue(mConfigPrefix + "Port" +
                                           toString(i), dfltPort.front());

        if (server != "")
        {
            mServers.push_back(server);
            mPorts.push_back(port);
        }
    }
    if (mServers.size() == 0)
    {
        mServers.assign(dflt.begin(), dflt.end());
        mPorts.assign(dfltPort.begin(), dfltPort.end());
    }
}

void LoginDialog::DropDownList::save(const std::string &server,
                                     const std::string &port)
{
    int position = 0;
    saveEntry(server, port, position);
    for (std::vector<std::string>::const_iterator sPtr = mServers.begin(),
             sEnd = mServers.end(),
             pPtr = mPorts.begin(),
             pEnd = mPorts.end();
         sPtr != sEnd && pPtr != pEnd;
         ++sPtr, ++pPtr)
    {
        if (*sPtr != server || *pPtr != port)
        {
            saveEntry(*sPtr, *pPtr, position);
        }
    }
}

int LoginDialog::DropDownList::getNumberOfElements()
{
    return mServers.size();
}

std::string LoginDialog::DropDownList::getElementAt(int i)
{
    if (i < 0 || i >= getNumberOfElements())
    {
      return "";
    }
    return getServerAt(i) + ":" + getPortAt(i);
}

std::string LoginDialog::DropDownList::getServerAt(int i)
{
    if (i < 0 || i >= getNumberOfElements())
    {
        return "";
    }
    return mServers.at(i);
}

std::string LoginDialog::DropDownList::getPortAt(int i)
{
    if (i < 0 || i >= getNumberOfElements())
    {
        return "";
    }
    return mPorts.at(i);
}
