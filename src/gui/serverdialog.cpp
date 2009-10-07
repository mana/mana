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
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/textfield.h"

#include "configuration.h"
#include "log.h"
#include "main.h"

#include "net/net.h"

#include "utils/xml.h"
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
    std::string myServer = servers.at(elementIndex).name;
    myServer += " (";
    myServer += std::string(servers.at(elementIndex).hostname);
    myServer += ":";
    myServer += toString(servers.at(elementIndex).port);
    myServer += ")";
    return myServer;
}

void ServersListModel::addFirstElement(const ServerInfo &server)
{
    // Equivalent to push_front
    std::vector<ServerInfo>::iterator MyIterator = servers.begin();
    servers.insert(MyIterator, 1, server);
}

void ServersListModel::addElement(const ServerInfo &server)
{
    servers.push_back(server);
}

void ServersListModel::mergeElement(const ServerInfo &server)
{
    // search through the list
    for (int i = 0; i < getNumberOfElements(); i++)
    {
        // the server is already in the list, merge its properties
        if (servers[i] == server)
        {
            servers[i].name = server.name;
            return;
        }
    }
    // the server is not found, add it at the end of the list
    addElement(server);
}

bool ServersListModel::contains(const ServerInfo &server)
{
    // search through the list
    for (int i = 0; i < getNumberOfElements(); i++)
    {
        if (servers[i] == server)
            return true;
    }
    return false;
}


ServerDialog::ServerDialog(ServerInfo *serverInfo):
    Window(_("Choose Your Server")), mServerInfo(serverInfo)
{
    mServerDescription = new Label(std::string());
    gcn::Label *serverLabel = new Label(_("Server:"));
    gcn::Label *portLabel = new Label(_("Port:"));
    mServerNameField = new TextField(mServerInfo->hostname);
    mPortField = new TextField(toString(mServerInfo->port));

    mMostUsedServersListModel = new ServersListModel;
    ServerInfo currentServer;
    ServerInfo tempServer;

    // Add the most used servers from config if they are not in the online list
    std::string currentConfig = "";
    for (int i = 0; i <= MAX_SERVERLIST; i++)
    {
        currentServer.clear();

        currentConfig = "MostUsedServerName" + toString(i);
        currentServer.hostname = config.getValue(currentConfig, "");

        currentConfig = "MostUsedServerPort" + toString(i);
        currentServer.port = (short) config.getValue(currentConfig, DEFAULT_PORT);

        if (!currentServer.hostname.empty() && currentServer.port != 0)
        {
            if (!mMostUsedServersListModel->contains(currentServer))
                mMostUsedServersListModel->addElement(currentServer);
        }
    }

    // load a list with online servers...
    loadServerlist();

    mMostUsedServersList = new ListBox(mMostUsedServersListModel);
    ScrollArea *usedScroll = new ScrollArea(mMostUsedServersList);
    usedScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mQuitButton = new Button(_("Quit"), "quit", this);
    mConnectButton = new Button(_("Connect"), "connect", this);
    mManualEntryButton = new Button(_("Add Entry"), "addEntry", this);

    mServerNameField->setActionEventId("connect");
    mPortField->setActionEventId("connect");

    mServerNameField->addActionListener(this);
    mPortField->addActionListener(this);
    mManualEntryButton->addActionListener(this);
    mMostUsedServersList->addSelectionListener(this);
    mMostUsedServersList->setSelected(0);
    usedScroll->setVerticalScrollAmount(0);

    place(0, 0, mServerDescription, 2);
    place(0, 1, serverLabel);
    place(0, 2, portLabel);
    place(1, 1, mServerNameField, 3).setPadding(3);
    place(1, 2, mPortField, 3).setPadding(3);
    place(0, 3, usedScroll, 4, 5).setPadding(3);
    place(0, 8, mManualEntryButton);
    place(2, 8, mQuitButton);
    place(3, 8, mConnectButton);

    // Make sure the list has enough height
    getLayout().setRowHeight(3, 80);


    reflowLayout(300, 0);

    center();
    setFieldsReadOnly(true);
    setVisible(true);

    if (mServerNameField->getText().empty()) {
        mServerNameField->requestFocus();
    } else {
        if (mPortField->getText().empty()) {
            mPortField->requestFocus();
        } else {
            mConnectButton->requestFocus();
        }
    }

}

ServerDialog::~ServerDialog()
{
    delete mMostUsedServersListModel;
}

void ServerDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        // Give focus back to the server dialog.
        mServerNameField->requestFocus();
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
            mQuitButton->setEnabled(false);
            mConnectButton->setEnabled(false);

            // First, look if the entry is a new one.
            ServerInfo currentServer;
            ServerInfo tempServer;
            currentServer.hostname = mServerNameField->getText();
            currentServer.port = (short) atoi(mPortField->getText().c_str());

            // now rewrite the configuration...
            // id = 0 is always the last selected server
            config.setValue("MostUsedServerName0", currentServer.hostname);
            config.setValue("MostUsedServerPort0", currentServer.port);

            // now add the rest of the list...
            std::string currentConfig = "";
            int configCount = 1;
            for (int i = 0; i < mMostUsedServersListModel->getNumberOfElements(); i++)
            {
                tempServer = mMostUsedServersListModel->getServer(i);

                // ensure, that our server will not be added twice
                if (tempServer != currentServer)
                {
                    currentConfig = "MostUsedServerName" + toString(configCount);
                    config.setValue(currentConfig, toString(tempServer.hostname));
                    currentConfig = "MostUsedServerPort" + toString(configCount);
                    config.setValue(currentConfig, toString(tempServer.port));
                    configCount++;
                }

                // stop if we exceed the number of maximum config entries
                if (configCount >= MAX_SERVERLIST)
                    break;
            }
            mServerInfo->hostname = currentServer.hostname;
            mServerInfo->port = currentServer.port;
            state = STATE_CONNECT_SERVER;
        }
    }
    else if (event.getId() == "quit")
    {
        state = STATE_FORCE_QUIT;
    }
    else if (event.getId() == "addEntry")
    {
        setFieldsReadOnly(false);
    }
}

void ServerDialog::valueChanged(const gcn::SelectionEvent &event)
{
    const int index = mMostUsedServersList->getSelected();
    if (index == -1)
        return;

    // Update the server and post fields according to the new selection
    const ServerInfo myServer = mMostUsedServersListModel->getServer(index);
    mServerDescription->setCaption(myServer.name);
    mServerNameField->setText(myServer.hostname);
    mPortField->setText(toString(myServer.port));

    setFieldsReadOnly(true);
}

void ServerDialog::loadServerlist()
{
    ServerInfo currentServer;
    currentServer.clear();

    // try to load the configuration value for the onlineServerList
    std::string listFile = config.getValue("onlineServerList", "void");
    // if there is no entry, try to load the file from the default updatehost
    if (listFile == "void")
        listFile = config.getValue("updatehost", "http://updates.themanaworld.org")
            + "/serverlist.xml";

    xmlDocPtr doc = xmlReadFile(listFile.c_str(), NULL, 0);
    if (doc == NULL)
    {
        logger->log("Failed to load online serverlist from %s", listFile.c_str());
        return;
    }

    xmlNodePtr rootNode = xmlDocGetRootElement(doc);
    int version = XML::getProperty(rootNode, "version", 3);

    if (version != 1)
    {
        logger->log("Online server list has wrong version");
        return;
    }

    for_each_xml_child_node(server, rootNode)
    {
        if (xmlStrEqual(server->name, BAD_CAST "server"))
        {
            //check wether the version matches
            #ifdef TMWSERV_SUPPORT
            if (XML::getProperty(server, "type", "unknown") != "TMWSERV")
                continue;
            #endif

            #ifdef EATHENA_SUPPORT
            if (XML::getProperty(server, "type", "unknown") != "EATHENA")
                continue;
            #endif

            currentServer.clear();
            currentServer.name = XML::getProperty(server, "name", std::string());

            for_each_xml_child_node(subnode, server)
            {
                if (xmlStrEqual(subnode->name, BAD_CAST "connection"))
                {
                    currentServer.hostname = XML::getProperty(subnode, "hostname", std::string());
                    currentServer.port = XML::getProperty(subnode, "port", DEFAULT_PORT);
                }
            }

            // merge the server into the local list
            mMostUsedServersListModel->mergeElement(currentServer);
        }
    }

    xmlFreeDoc(doc);
}

void ServerDialog::setFieldsReadOnly(const bool readOnly)
{
    if (readOnly)
    {
        mServerNameField->setEnabled(false);
        mPortField->setEnabled(false);
        mManualEntryButton->setVisible(true);
        mServerDescription->setVisible(true);
    }
    else
    {
        mManualEntryButton->setVisible(false);

        mServerDescription->setVisible(false);
        mServerDescription->setCaption(std::string());
        mMostUsedServersList->setSelected(-1);

        mServerNameField->setText(std::string());
        mServerNameField->setEnabled(true);

        mPortField->setText(toString(DEFAULT_PORT));
        mPortField->setEnabled(true);

        mServerNameField->requestFocus();
    }
}


