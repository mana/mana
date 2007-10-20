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

#include "serverdialog.h"

#include <iostream>
#include <string>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "listbox.h"
#include "ok_dialog.h"
#include "scrollarea.h"
#include "textfield.h"

#include "widgets/layout.h"

#include "../configuration.h"
#include "../log.h"
#include "../logindata.h"
#include "../main.h"

#include "../utils/gettext.h"
#include "../utils/tostring.h"

const short MAX_SERVERLIST = 5;

void
DropDownListener::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        // Reset the text fields and give back the server dialog.
        mServerNameField->setText("");
        mServerNameField->setCaretPosition(0);
        mServerPortField->setText("");
        mServerPortField->setCaretPosition(0);

        mServerNameField->requestFocus();
    }
    else if (event.getId() == "changeSelection")
    {
        // Change the textField Values according to new selection
        if (currentSelectedIndex != mServersListBox->getSelected())
        {
            Server myServer;
            myServer = mServersListModel->getServer(
                    mServersListBox->getSelected());
            mServerNameField->setText(myServer.serverName);
            mServerPortField->setText(toString(myServer.port));
            currentSelectedIndex = mServersListBox->getSelected();
        }
    }
}

int ServersListModel::getNumberOfElements()
{
    return servers.size();
}

std::string ServersListModel::getElementAt(int elementIndex)
{
    std::string myServer = "";
    myServer = servers.at(elementIndex).serverName;
    myServer += ":";
    myServer += toString(servers.at(elementIndex).port);
    return myServer;
}

void ServersListModel::addFirstElement(Server server)
{
    // Equivalent to push_front
    std::vector<Server>::iterator MyIterator = servers.begin();
    servers.insert(MyIterator, 1, server);
}

void ServersListModel::addElement(Server server)
{
    servers.push_back(server);
}

ServerDialog::ServerDialog(LoginData *loginData):
    Window(_("Choose your Mana World Server")), mLoginData(loginData)
{
    gcn::Label *serverLabel = new gcn::Label(_("Server:"));
    gcn::Label *portLabel = new gcn::Label(_("Port:"));
    mServerNameField = new TextField(mLoginData->hostname);
    mPortField = new TextField(toString(mLoginData->port));

    // Add the most used servers from config
    mMostUsedServersListModel = new ServersListModel();
    Server currentServer;
    std::string currentConfig = "";
    for (int i=0; i<=MAX_SERVERLIST; i++)
    {
        currentServer.serverName = "";
        currentServer.port = 0;

        currentConfig = "MostUsedServerName" + toString(i);
        currentServer.serverName = config.getValue(currentConfig, "");

        currentConfig = "MostUsedServerPort" + toString(i);
        currentServer.port = (short)atoi(config.getValue(currentConfig, "").c_str());
        if (!currentServer.serverName.empty() || currentServer.port != 0)
        {
            mMostUsedServersListModel->addElement(currentServer);
        }
    }

    mMostUsedServersListBox = new ListBox(NULL);
    mMostUsedServersListBox->setListModel(mMostUsedServersListModel);
    mMostUsedServersScrollArea = new ScrollArea();
    mMostUsedServersDropDown = new DropDown(mMostUsedServersListModel,
        mMostUsedServersScrollArea, mMostUsedServersListBox);

    mDropDownListener = new DropDownListener(mServerNameField, mPortField,
            mMostUsedServersListModel, mMostUsedServersListBox);

    mOkButton = new Button(_("Ok"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    mServerNameField->setActionEventId("ok");
    mPortField->setActionEventId("ok");
    mMostUsedServersDropDown->setActionEventId("changeSelection");

    mServerNameField->addActionListener(this);
    mPortField->addActionListener(this);
    mMostUsedServersDropDown->addActionListener(mDropDownListener);

    setPadding(8);
    place(0, 0, serverLabel);
    place(0, 1, portLabel);
    place(1, 0, mServerNameField, 3).setPadding(3);
    place(1, 1, mPortField, 3).setPadding(3);
    place(0, 2, mMostUsedServersDropDown, 4).setPadding(3);
    place(2, 3, mOkButton);
    place(3, 3, mCancelButton);
    Layout &layout = getLayout();
    layout.setWidth(250);
    layout.setColWidth(1, Layout::FILL);
    reflowLayout();
    forgetLayout();

    setLocationRelativeTo(getParent());
    setVisible(true);

    if (mServerNameField->getText().empty()) {
        mServerNameField->requestFocus();
    } else {
        if (mPortField->getText().empty()) {
            mPortField->requestFocus();
        } else {
            mOkButton->requestFocus();
        }
    }
}

ServerDialog::~ServerDialog()
{
    delete mDropDownListener;
}

void
ServerDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        // Check login
        if (mServerNameField->getText().empty() || mPortField->getText().empty())
        {
            OkDialog *dlg = new OkDialog(_("Error"), "Enter the chosen server.");
            dlg->addActionListener(mDropDownListener);
        }
        else
        {
            mLoginData->hostname = mServerNameField->getText();
            mLoginData->port = (short) atoi(mPortField->getText().c_str());
            mOkButton->setEnabled(false);
            mCancelButton->setEnabled(false);

            // First, look if the entry is a new one.
            Server currentServer;
            bool newEntry = true;
            for (int i = 0; i < mMostUsedServersListModel->getNumberOfElements(); i++)
            {
                currentServer = mMostUsedServersListModel->getServer(i);
                if (currentServer.serverName == mLoginData->hostname &&
                    currentServer.port == mLoginData->port)
                    newEntry = false;
            }
            // Then, add it to config if it's really new
            currentServer.serverName = mLoginData->hostname;
            currentServer.port = mLoginData->port;
            if (newEntry)
                mMostUsedServersListModel->addFirstElement(currentServer);
            // Write the entry in config
            std::string currentConfig = "";
            for (int i = 0; i < mMostUsedServersListModel->getNumberOfElements(); i++)
            {
                currentServer = mMostUsedServersListModel->getServer(i);

                currentConfig = "MostUsedServerName" + toString(i);
                config.setValue(currentConfig, currentServer.serverName);

                currentConfig = "MostUsedServerPort" + toString(i);
                config.setValue(currentConfig, toString(currentServer.port));
            }
            state = STATE_CONNECT_ACCOUNT;
        }
    }
    else if (event.getId() == "cancel")
    {
        state = STATE_FORCE_QUIT;
    }
}
