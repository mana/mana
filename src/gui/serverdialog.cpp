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

#include "gui/serverdialog.h"

#include "gui/okdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/dropdown.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/textfield.h"

#include "configuration.h"
#include "log.h"
#include "main.h"

#include "net/net.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <cstdlib>
#include <iostream>
#include <string>

const short MAX_SERVERLIST = 5;

int ServersListModel::getNumberOfElements()
{
    return servers.size();
}

std::string ServersListModel::getElementAt(int elementIndex)
{
    std::string myServer = std::string(servers.at(elementIndex).hostname);
    myServer += ":";
    myServer += toString(servers.at(elementIndex).port);
    return myServer;
}

void ServersListModel::addFirstElement(ServerInfo server)
{
    // Equivalent to push_front
    std::vector<ServerInfo>::iterator MyIterator = servers.begin();
    servers.insert(MyIterator, 1, server);
}

void ServersListModel::addElement(ServerInfo server)
{
    servers.push_back(server);
}

ServerDialog::ServerDialog(ServerInfo *serverInfo):
    Window(_("Choose Your Server")), mServerInfo(serverInfo)
{
    gcn::Label *serverLabel = new Label(_("Server:"));
    gcn::Label *portLabel = new Label(_("Port:"));
    mServerNameField = new TextField(mServerInfo->hostname);
    mPortField = new TextField(toString(mServerInfo->port));

    // Add the most used servers from config
    mMostUsedServersListModel = new ServersListModel;
    ServerInfo currentServer;
    std::string currentConfig = "";
    for (int i=0; i<=MAX_SERVERLIST; i++)
    {
        currentServer.clear();

        currentConfig = "MostUsedServerName" + toString(i);
        currentServer.hostname = config.getValue(currentConfig, "");

        currentConfig = "MostUsedServerPort" + toString(i);
        currentServer.port = (short)atoi(config.getValue(currentConfig, "").c_str());
        if (!currentServer.hostname.empty() || currentServer.port != 0)
        {
            mMostUsedServersListModel->addElement(currentServer);
        }
    }

    mMostUsedServersDropDown = new DropDown(mMostUsedServersListModel);

    mOkButton = new Button(_("OK"), "connect", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    mServerNameField->setActionEventId("connect");
    mPortField->setActionEventId("connect");
    mMostUsedServersDropDown->setActionEventId("changeSelection");

    mServerNameField->addActionListener(this);
    mPortField->addActionListener(this);
    mMostUsedServersDropDown->addActionListener(this);

    mMostUsedServersDropDown->setSelected(0);

    place(0, 0, serverLabel);
    place(0, 1, portLabel);
    place(1, 0, mServerNameField, 3).setPadding(2);
    place(1, 1, mPortField, 3).setPadding(2);
    place(0, 2, mMostUsedServersDropDown, 4).setPadding(2);
    // TODO: Find a better way to give the dropdown window more room
    place(2, 12, mOkButton);
    place(3, 12, mCancelButton);
    reflowLayout(250, 0);

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
    delete mMostUsedServersListModel;
}

void
ServerDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        // Give focus back to the server dialog.
        mServerNameField->requestFocus();
    }
    else if (event.getId() == "changeSelection")
    {
        // Change the textField Values according to new selection
        ServerInfo myServer = mMostUsedServersListModel->getServer
            (mMostUsedServersDropDown->getSelected());
        mServerNameField->setText(myServer.hostname);
        mPortField->setText(toString(myServer.port));
    }
    else if (event.getId() == "connect")
    {
        // Check login
        if (mServerNameField->getText().empty() || mPortField->getText().empty())
        {
            OkDialog *dlg = new OkDialog(_("Error"),
                _("Please type both the address and the port of a server."));
            dlg->addActionListener(this);
        }
        else
        {
            mOkButton->setEnabled(false);
            mCancelButton->setEnabled(false);

            // First, look if the entry is a new one.
            ServerInfo currentServer;
            ServerInfo tempServer;
            currentServer.hostname = mServerNameField->getText();
            currentServer.port = (short) atoi(mPortField->getText().c_str());
            bool newEntry = true;
            for (int i = 0; i < mMostUsedServersListModel->getNumberOfElements(); i++)
            {
                tempServer = mMostUsedServersListModel->getServer(i);
                if (tempServer.hostname == mServerInfo->hostname &&
                    tempServer.port == mServerInfo->port)
                    newEntry = false;
            }
            if (newEntry)
                mMostUsedServersListModel->addFirstElement(currentServer);
            // Write the entry in config
            std::string currentConfig = "";
            for (int i = 0; i < mMostUsedServersListModel->getNumberOfElements(); i++)
            {
                tempServer = mMostUsedServersListModel->getServer(i);

                currentConfig = "MostUsedServerName" + toString(i);
                config.setValue(currentConfig, tempServer.hostname);

                currentConfig = "MostUsedServerPort" + toString(i);
                config.setValue(currentConfig, toString(tempServer.port));
            }
            mServerInfo->hostname = currentServer.hostname;
            mServerInfo->port = currentServer.port;
            state = STATE_CONNECT_SERVER;
        }
    }
    else if (event.getId() == "cancel")
    {
        state = STATE_FORCE_QUIT;
    }
}
