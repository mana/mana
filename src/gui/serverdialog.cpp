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

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/xml.h"

#include <cstdlib>
#include <iostream>
#include <string>

#define MAX_SERVERLIST 5

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
        currentServer.port = (short) config.getValue(currentConfig, DEFAULT_PORT);

        if (!currentServer.hostname.empty() && currentServer.port != 0)
        {
            mServers.push_back(currentServer);
        }
    }

    mServersListModel = new ServersListModel(&mServers, this);

    mServersList = new ListBox(mServersListModel);
    ScrollArea *usedScroll = new ScrollArea(mServersList);
    usedScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

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
    place(0, 2, usedScroll, 4, 5).setPadding(3);
    place(0, 7, mDescription, 4);
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

    downloadServerList();
}

ServerDialog::~ServerDialog()
{
    delete mServersListModel;
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

            // now rewrite the configuration...
            // id = 0 is always the last selected server
            config.setValue("MostUsedServerName0", currentServer.hostname);
            config.setValue("MostUsedServerPort0", currentServer.port);

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
        mDownload->cancel();
        state = STATE_FORCE_QUIT;
    }
    else if (event.getId() == "addEntry")
    {
        setFieldsReadOnly(false);
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
    // try to load the configuration value for the onlineServerList
    std::string listFile = config.getValue("onlineServerList", "void");
    // if there is no entry, try to load the file from the default updatehost
    if (listFile == "void")
        listFile = config.getValue("updatehost", "http://updates.themanaworld.org")
            + "/serverlist.xml";

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
                //check wether the version matches
                #ifdef MANASERV_SUPPORT
                if (XML::getProperty(server, "type", "unknown") != "MANASERV")
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
