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

ServerInfo::Type stringToServerType(const std::string &type)
{
    if (compareStrI(type, "eathena") == 0)
    {
        return ServerInfo::EATHENA;
    }
    else if (compareStrI(type, "manaserv") == 0)
    {
        return ServerInfo::MANASERV;
    }
    else
    {
        return ServerInfo::UNKNOWN;
    }
}

std::string serverTypeToString(ServerInfo::Type type)
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

    ServerInfo currentServer;
    // Add the most used servers from config if they are not in the online list
    std::string currentConfig = "";
    for (int i = 0; i <= MAX_SERVERLIST; i++)
    {
        currentServer.clear();

        currentConfig = "MostUsedServerName" + toString(i);
        currentServer.hostname = config.getValue(currentConfig, "");

        currentConfig = "MostUsedServerPort" + toString(i);
        currentServer.port = (short) config.getValue(currentConfig,
                                                     DEFAULT_PORT);

        currentConfig = "MostUsedServerType" + toString(i);
        currentServer.type = stringToServerType(config
                                                .getValue(currentConfig, ""));

        if (!currentServer.hostname.empty() && currentServer.port != 0)
        {
            mServers.push_back(currentServer);
        }
    }

    mServersListModel = new ServersListModel(&mServers, this);

    mServersList = new ListBox(mServersListModel);
    ScrollArea *usedScroll = new ScrollArea(mServersList);
    usedScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mTypeListModel = new TypeListModel();
    mTypeField = new DropDown(mTypeListModel);

    mDescription = new Label(std::string());

    mQuitButton = new Button(_("Quit"), "quit", this);
    mConnectButton = new Button(_("Connect"), "connect", this);
    mManualEntryButton = new Button(_("Add Entry"), "addEntry", this);

    mServerNameField->setActionEventId("connect");
    mPortField->setActionEventId("connect");

    mServerNameField->addActionListener(this);
    mPortField->addActionListener(this);
    mManualEntryButton->addActionListener(this);
    mServersList->addSelectionListener(this);
    mServersList->setSelected(0);
    usedScroll->setVerticalScrollAmount(0);

    place(0, 0, serverLabel);
    place(1, 0, mServerNameField, 3).setPadding(3);
    place(0, 1, portLabel);
    place(1, 1, mPortField, 3).setPadding(3);
    place(0, 2, typeLabel);
    place(1, 2, mTypeField, 3).setPadding(3);
    place(0, 3, usedScroll, 4, 5).setPadding(3);
    place(0, 8, mDescription, 4);
    place(0, 9, mManualEntryButton);
    place(2, 9, mQuitButton);
    place(3, 9, mConnectButton);

    // Make sure the list has enough height
    getLayout().setRowHeight(3, 80);

    reflowLayout(300, 0);

    addKeyListener(this);

    center();
    setFieldsReadOnly(true);
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
        if (mServerNameField->getText().empty() || mPortField->getText().empty())
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
            ServerInfo tempServer;
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
            std::string currentConfig = "";
            int configCount = 1;
            for (int i = 0; i < mServersListModel->getNumberOfElements(); i++)
            {
                tempServer = mServersListModel->getServer(i);

                // ensure, that our server will not be added twice
                if (tempServer != currentServer)
                {
                    currentConfig = "MostUsedServerName" + toString(configCount);
                    config.setValue(currentConfig, toString(tempServer.hostname));
                    currentConfig = "MostUsedServerPort" + toString(configCount);
                    config.setValue(currentConfig, toString(tempServer.port));
                    currentConfig = "MostUsedServerType" + toString(configCount);
                    config.setValue(currentConfig, serverTypeToString(tempServer.type));
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

void ServerDialog::valueChanged(const gcn::SelectionEvent &event)
{
    const int index = mServersList->getSelected();
    if (index == -1)
        return;

    // Update the server and post fields according to the new selection
    const ServerInfo myServer = mServersListModel->getServer(index);
    mDescription->setCaption(myServer.name);
    mServerNameField->setText(myServer.hostname);
    mPortField->setText(toString(myServer.port));
    switch (myServer.type)
    {
        case ServerInfo::UNKNOWN:
            mTypeField->setSelected(2);
            break;
        case ServerInfo::EATHENA:
            mTypeField->setSelected(0);
            break;
        case ServerInfo::MANASERV:
            mTypeField->setSelected(1);
    }
    setFieldsReadOnly(true);
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

void ServerDialog::setFieldsReadOnly(const bool readOnly)
{
    if (readOnly)
    {
        mServerNameField->setEnabled(false);
        mPortField->setEnabled(false);
        mManualEntryButton->setVisible(true);
        mDescription->setVisible(true);
    }
    else
    {
        mManualEntryButton->setVisible(false);

        mDescription->setVisible(false);
        mDescription->setCaption(std::string());
        mServersList->setSelected(-1);

        mServerNameField->setText(std::string());
        mServerNameField->setEnabled(true);

        mPortField->setText(toString(DEFAULT_PORT));
        mPortField->setEnabled(true);

        mServerNameField->requestFocus();
    }
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
    ServerInfo currentServer;

    xmlDocPtr doc = xmlReadFile((mDir + "/serverlist.xml").c_str(), NULL, 0);

    if (doc != NULL)
    {
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
                std::string type = XML::getProperty(server, "type", "unknown");

                currentServer.clear();
                currentServer.name = XML::getProperty(server, "name", std::string());

                for_each_xml_child_node(subnode, server)
                {
                    if (xmlStrEqual(subnode->name, BAD_CAST "connection"))
                    {
                        currentServer.type = stringToServerType(type);
                        currentServer.hostname = XML::getProperty(subnode, "hostname", std::string());
                        currentServer.port = XML::getProperty(subnode, "port", DEFAULT_PORT);
                    }
                }


                MutexLocker lock(&mMutex);
                // add the server to the local list (if it's not already present)
                ServerInfos::iterator it;
                bool found = false;
                for (it = mServers.begin(); it != mServers.end(); it++)
                {
                    if ((*it) == currentServer)
                    {
                        (*it).name = currentServer.name;
                        found = true;
                        break;
                    }
                }

                if (!found)
                    mServers.push_back(currentServer);
            }
        }

        xmlFreeDoc(doc);
    }

    MutexLocker lock(&mMutex);
    mDownloadStatus = DOWNLOADING_COMPLETE;
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

        if (progress != progress) progress = 0.0f; // check for NaN
        if (progress < 0.0f) progress = 0.0f; // no idea how this could ever happen, but why not check for it anyway.
        if (progress > 1.0f) progress = 1.0f;

        MutexLocker lock(&sd->mMutex);
        sd->mDownloadStatus = DOWNLOADING_IN_PROGRESS;
        sd->mDownloadProgress = progress;
    }

    if (finished)
    {
        sd->loadServers();
    }

    return 0;
}
