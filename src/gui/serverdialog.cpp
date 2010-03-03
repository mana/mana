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

#include "gui/serverdialog.h"

#include "client.h"
#include "configuration.h"
#include "log.h"

#include "gui/okdialog.h"
#include "gui/sdlinput.h"

#include "gui/widgets/button.h"
#include "gui/widgets/dropdown.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/textfield.h"

#include "net/net.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/xml.h"
#include "widgets/dropdown.h"

#include <cstdlib>
#include <iostream>
#include <string>

#define MAX_SERVERLIST 5

static ServerInfo::Type stringToServerType(const std::string &type)
{
    if (compareStrI(type, "eathena") == 0)
        return ServerInfo::EATHENA;
    else if (compareStrI(type, "manaserv") == 0)
        return ServerInfo::MANASERV;

    return ServerInfo::UNKNOWN;
}

static std::string serverTypeToString(ServerInfo::Type type)
{
    switch (type)
    {
    case ServerInfo::EATHENA:
        return "eAthena";
    case ServerInfo::MANASERV:
        return "manaserv";
    default:
        return "";
    }
}

static unsigned short defaultPortForServerType(ServerInfo::Type type)
{
    switch (type)
    {
    default:
    case ServerInfo::EATHENA:
        return 6901;
    case ServerInfo::MANASERV:
        return 9601;
    }
}

ServersListModel::ServersListModel(ServerInfos *servers, ServerDialog *parent):
        mServers(servers),
        mParent(parent)
{
}

int ServersListModel::getNumberOfElements()
{
    MutexLocker lock = mParent->lock();
    return mServers->size();
}

std::string ServersListModel::getElementAt(int elementIndex)
{
    MutexLocker lock = mParent->lock();
    ServerInfo server = mServers->at(elementIndex);
    std::string myServer;
    if (server.name.empty())
    {
        myServer += server.hostname;
        myServer += ":";
        myServer += toString(server.port);
    }
    else
    {
        myServer += server.name;
        myServer += " (";
        myServer += server.hostname;
        myServer += ":";
        myServer += toString(server.port);
        myServer += ")";
    }
    return myServer;
}

std::string TypeListModel::getElementAt(int elementIndex)
{
    if (elementIndex == 0)
        return "eAthena";
    else if (elementIndex == 1)
        return "Manaserv";
    else
        return "Unknown";
}


ServerDialog::ServerDialog(ServerInfo *serverInfo, const std::string &dir):
    Window(_("Choose Your Server")),
    mDir(dir),
    mDownloadStatus(DOWNLOADING_PREPARING),
    mDownloadProgress(-1.0f),
    mServers(ServerInfos()),
    mServerInfo(serverInfo)
{
    Label *serverLabel = new Label(_("Server:"));
    Label *portLabel = new Label(_("Port:"));
    Label *typeLabel = new Label(_("Server type:"));
    mServerNameField = new TextField(mServerInfo->hostname);
    mPortField = new TextField(toString(mServerInfo->port));

    // Add the most used servers from config
    for (int i = 0; i <= MAX_SERVERLIST; ++i)
    {
        const std::string index = toString(i);
        const std::string nameKey = "MostUsedServerName" + index;
        const std::string typeKey = "MostUsedServerType" + index;
        const std::string portKey = "MostUsedServerPort" + index;

        ServerInfo server;
        server.hostname = config.getValue(nameKey, "");
        server.type = stringToServerType(config.getValue(typeKey, ""));

        const int defaultPort = defaultPortForServerType(server.type);
        server.port = (unsigned short) config.getValue(portKey, defaultPort);

        if (server.isValid())
        {
            server.save = true;
            mServers.push_back(server);
        }
    }

    mServersListModel = new ServersListModel(&mServers, this);

    mServersList = new ListBox(mServersListModel);
    mServersList->addMouseListener(this);

    ScrollArea *usedScroll = new ScrollArea(mServersList);
    usedScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mTypeListModel = new TypeListModel();
    mTypeField = new DropDown(mTypeListModel);

    mDescription = new Label(std::string());

    mQuitButton = new Button(_("Quit"), "quit", this);
    mConnectButton = new Button(_("Connect"), "connect", this);
    mManualEntryButton = new Button(_("Custom Server"), "addEntry", this);
    mDeleteButton = new Button(_("Delete"), "remove", this);

    mServerNameField->setActionEventId("connect");
    mPortField->setActionEventId("connect");

    mServerNameField->addActionListener(this);
    mPortField->addActionListener(this);
    mManualEntryButton->addActionListener(this);
    mServersList->addSelectionListener(this);
    usedScroll->setVerticalScrollAmount(0);

    place(0, 0, serverLabel);
    place(1, 0, mServerNameField, 4).setPadding(3);
    place(0, 1, portLabel);
    place(1, 1, mPortField, 4).setPadding(3);
    place(0, 2, typeLabel);
    place(1, 2, mTypeField, 4).setPadding(3);
    place(0, 3, usedScroll, 5, 5).setPadding(3);
    place(0, 8, mDescription, 5);
    place(0, 9, mManualEntryButton);
    place(1, 9, mDeleteButton);
    place(3, 9, mQuitButton);
    place(4, 9, mConnectButton);

    // Make sure the list has enough height
    getLayout().setRowHeight(3, 80);

    reflowLayout(300, 0);

    addKeyListener(this);

    center();
    setFieldsReadOnly(true);
    mServersList->setSelected(0); // Do this after for the Delete button
    setVisible(true);

    if (mServerNameField->getText().empty())
    {
        mServerNameField->requestFocus();
    }
    else
    {
        if (mPortField->getText().empty())
            mPortField->requestFocus();
        else
            mConnectButton->requestFocus();
    }

    downloadServerList();
}

ServerDialog::~ServerDialog()
{
    if (mDownload)
    {
        mDownload->cancel();
        delete mDownload;
        mDownload = 0;
    }
    delete mServersListModel;
    delete mTypeListModel;
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
        if (mServerNameField->getText().empty()
            || mPortField->getText().empty())
        {
            OkDialog *dlg = new OkDialog(_("Error"),
                _("Please type both the address and the port of a server."));
            dlg->addActionListener(this);
        }
        else
        {
            mDownload->cancel();
            mQuitButton->setEnabled(false);
            mConnectButton->setEnabled(false);

            // First, look if the entry is a new one.
            ServerInfo currentServer;
            currentServer.hostname = mServerNameField->getText();
            currentServer.port = (short) atoi(mPortField->getText().c_str());
            switch (mTypeField->getSelected())
            {
                case 0:
                    currentServer.type = ServerInfo::EATHENA;
                    break;
                case 1:
                    currentServer.type = ServerInfo::MANASERV;
                    break;
                default:
                    currentServer.type = ServerInfo::UNKNOWN;
            }

            // now rewrite the configuration...
            // id = 0 is always the last selected server
            config.setValue("MostUsedServerName0", currentServer.hostname);
            config.setValue("MostUsedServerPort0", currentServer.port);
            config.setValue("MostUsedServerType0",
                            serverTypeToString(currentServer.type));

            // now add the rest of the list...
            int configCount = 1;
            for (int i = 0; i < mServersListModel->getNumberOfElements(); ++i)
            {
                const ServerInfo server = mServersListModel->getServer(i);

                // Only save servers that were loaded from settings
                if (!server.save)
                    continue;

                // ensure, that our server will not be added twice
                if (server != currentServer)
                {
                    const std::string index = toString(configCount);
                    const std::string nameKey = "MostUsedServerName" + index;
                    const std::string typeKey = "MostUsedServerType" + index;
                    const std::string portKey = "MostUsedServerPort" + index;

                    config.setValue(nameKey, toString(server.hostname));
                    config.setValue(typeKey, serverTypeToString(server.type));
                    config.setValue(portKey, toString(server.port));

                    configCount++;
                }

                // stop if we exceed the number of maximum config entries
                if (configCount >= MAX_SERVERLIST)
                    break;
            }
            mServerInfo->hostname = currentServer.hostname;
            mServerInfo->port = currentServer.port;
            mServerInfo->type = currentServer.type;
            Client::setState(STATE_CONNECT_SERVER);
        }
    }
    else if (event.getId() == "quit")
    {
        mDownload->cancel();
        Client::setState(STATE_FORCE_QUIT);
    }
    else if (event.getId() == "addEntry")
    {
        setFieldsReadOnly(false);
    }
    else if (event.getId() == "remove")
    {
        int index = mServersList->getSelected();
        mServersList->setSelected(0);
        mServersListModel->remove(index);
    }
}

void ServerDialog::keyPressed(gcn::KeyEvent &keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ESCAPE)
    {
        Client::setState(STATE_EXIT);
    }
    else if (key.getValue() == Key::ENTER)
    {
        action(gcn::ActionEvent(NULL, mConnectButton->getActionEventId()));
    }
}

void ServerDialog::valueChanged(const gcn::SelectionEvent &)
{
    const int index = mServersList->getSelected();
    if (index == -1)
    {
        mDeleteButton->setEnabled(false);
        return;
    }

    // Update the server and post fields according to the new selection
    const ServerInfo myServer = mServersListModel->getServer(index);
    mDescription->setCaption(myServer.name);
    mServerNameField->setText(myServer.hostname);
    mPortField->setText(toString(myServer.port));
    switch (myServer.type)
    {
        case ServerInfo::EATHENA:
        case ServerInfo::UNKNOWN:
            mTypeField->setSelected(0);
            break;
        case ServerInfo::MANASERV:
            mTypeField->setSelected(1);
            break;
    }
    setFieldsReadOnly(true);

    mDeleteButton->setEnabled(myServer.save);
}

void ServerDialog::mouseClicked(gcn::MouseEvent &mouseEvent)
{
    if (mouseEvent.getClickCount() == 2 &&
        mouseEvent.getSource() == mServersList)
    {
        action(gcn::ActionEvent(mConnectButton,
                                mConnectButton->getActionEventId()));
    }
}

void ServerDialog::logic()
{
    {
        MutexLocker lock(&mMutex);
        if (mDownloadStatus == DOWNLOADING_COMPLETE)
        {
            mDownloadStatus = DOWNLOADING_OVER;

            mDescription->setCaption(std::string());
        }
        else if (mDownloadStatus == DOWNLOADING_IN_PROGRESS)
        {
            mDescription->setCaption(strprintf(_("Downloading server list..."
                                                 "%2.2f%%"),
                                     mDownloadProgress * 100));
        }
        else if (mDownloadStatus == DOWNLOADING_IDLE)
        {
            mDescription->setCaption(_("Waiting for server..."));
        }
        else if (mDownloadStatus == DOWNLOADING_PREPARING)
        {
            mDescription->setCaption(_("Preparing download"));
        }
    }

    Window::logic();
}

void ServerDialog::setFieldsReadOnly(bool readOnly)
{
    if (!readOnly)
    {
        mDescription->setCaption(std::string());
        mServersList->setSelected(-1);

        mServerNameField->setText(std::string());
        mPortField->setText(std::string());

        mServerNameField->requestFocus();
    }

    mManualEntryButton->setEnabled(readOnly);
    mDeleteButton->setEnabled(false);
    mDescription->setVisible(readOnly);

    mServerNameField->setEnabled(!readOnly);
    mPortField->setEnabled(!readOnly);
    mTypeField->setEnabled(!readOnly);
}

void ServerDialog::downloadServerList()
{
    // Try to load the configuration value for the onlineServerList
    std::string listFile = branding.getValue("onlineServerList", std::string());

    if (listFile.empty())
        listFile = config.getValue("onlineServerList", std::string());

    // Fall back to manasource.org when neither branding nor config set it
    if (listFile.empty())
        listFile = "http://manasource.org/serverlist.xml";

    mDownload = new Net::Download(this, listFile, &downloadUpdate);
    mDownload->setFile(mDir + "/serverlist.xml");
    mDownload->start();
}

void ServerDialog::loadServers()
{
    XML::Document doc(mDir + "/serverlist.xml", false);
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "serverlist"))
    {
        logger->log("Error loading server list!");
        return;
    }

    int version = XML::getProperty(rootNode, "version", 0);
    if (version != 1)
    {
        logger->log("Error: unsupported online server list version: %d",
                    version);
        return;
    }

    for_each_xml_child_node(serverNode, rootNode)
    {
        if (!xmlStrEqual(serverNode->name, BAD_CAST "server"))
            continue;

        ServerInfo server;

        std::string type = XML::getProperty(serverNode, "type", "unknown");

        server.type = stringToServerType(type);
        server.name = XML::getProperty(serverNode, "name", std::string());

        if (server.type == ServerInfo::UNKNOWN)
        {
            logger->log("Unknown server type: %s", type.c_str());
            continue;
        }

        for_each_xml_child_node(subNode, serverNode)
        {
            if (!xmlStrEqual(subNode->name, BAD_CAST "connection"))
                continue;

            server.hostname = XML::getProperty(subNode, "hostname", "");
            server.port = XML::getProperty(subNode, "port", 0);
            if (server.port == 0)
            {
                // If no port is given, use the default for the given type
                server.port = defaultPortForServerType(server.type);
            }
        }


        MutexLocker lock(&mMutex);
        // Add the server to the local list if it's not already present
        ServerInfos::iterator it;
        bool found = false;
        for (it = mServers.begin(); it != mServers.end(); it++)
        {
            if ((*it) == server)
            {
                // Use the name listed in the server list
                (*it).name = server.name;
                found = true;
                break;
            }
        }

        if (!found)
            mServers.push_back(server);
    }
}

int ServerDialog::downloadUpdate(void *ptr, DownloadStatus status,
                                 size_t total, size_t remaining)
{
    if (status == DOWNLOAD_STATUS_CANCELLED)
        return -1;

    ServerDialog *sd = reinterpret_cast<ServerDialog*>(ptr);
    bool finished = false;

    if (status == DOWNLOAD_STATUS_COMPLETE)
    {
        finished = true;
    }
    else if (status < 0)
    {
        logger->log("Error retreiving server list: %s\n",
                    sd->mDownload->getError());

        finished = true;
    }
    else
    {
        float progress = (float) remaining / total;

        if (progress != progress)
            progress = 0.0f; // check for NaN
        else if (progress < 0.0f)
            progress = 0.0f; // no idea how this could ever happen, but why not check for it anyway.
        else if (progress > 1.0f)
            progress = 1.0f;

        MutexLocker lock(&sd->mMutex);
        sd->mDownloadStatus = DOWNLOADING_IN_PROGRESS;
        sd->mDownloadProgress = progress;
    }

    if (finished)
    {
        sd->loadServers();

        MutexLocker lock(&sd->mMutex);
        sd->mDownloadStatus = DOWNLOADING_COMPLETE;
    }

    return 0;
}
