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

#include "gui/serverdialog.h"

#include "chatlogger.h"
#include "client.h"
#include "configuration.h"
#include "gui.h"
#include "log.h"
#include "main.h"

#include "gui/customserverdialog.h"
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

#include "resources/theme.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/xml.h"

#include <guichan/font.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

static const int MAX_SERVERLIST = 6;

static std::string serverTypeToString(ServerInfo::Type type)
{
    switch (type)
    {
    case ServerInfo::TMWATHENA:
        return "TmwAthena";
    case ServerInfo::MANASERV:
        return "ManaServ";
    default:
        return "";
    }
}

ServersListModel::ServersListModel(ServerInfos *servers, ServerDialog *parent):
        mServers(servers),
        mVersionStrings(servers->size(), VersionString(0, "")),
        mParent(parent)
{
}

int ServersListModel::getNumberOfElements()
{
    MutexLocker lock(mParent->getMutex());
    return mServers->size();
}

std::string ServersListModel::getElementAt(int elementIndex)
{
    MutexLocker lock(mParent->getMutex());
    const ServerInfo &server = mServers->at(elementIndex);
    std::string myServer;
    myServer += server.hostname;
    myServer += ":";
    myServer += toString(server.port);
    return myServer;
}

void ServersListModel::setVersionString(int index, const std::string &version)
{
    if (version.empty())
        mVersionStrings[index] = VersionString(0, "");
    else
    {
        int width = gui->getFont()->getWidth(version);
        mVersionStrings[index] = VersionString(width, version);
    }
}

class ServersListBox : public ListBox
{
public:
    ServersListBox(ServersListModel *model):
            ListBox(model)
    {
    }

    void draw(gcn::Graphics *graphics)
    {
        if (!mListModel)
            return;

        ServersListModel *model = static_cast<ServersListModel*>(mListModel);

        updateAlpha();

        graphics->setColor(Theme::getThemeColor(Theme::HIGHLIGHT,
                                                (int) (mAlpha * 255.0f)));
        graphics->setFont(getFont());

        const int height = getRowHeight();
        const gcn::Color unsupported =
                Theme::getThemeColor(Theme::SERVER_VERSION_NOT_SUPPORTED,
                                     (int) (mAlpha * 255.0f));

        // Draw filled rectangle around the selected list element
        if (mSelected >= 0)
            graphics->fillRectangle(gcn::Rectangle(0, height * mSelected,
                                                   getWidth(), height));

        // Draw the list elements
        for (int i = 0, y = 0; i < model->getNumberOfElements();
             ++i, y += height)
        {
            ServerInfo info = model->getServer(i);

            graphics->setColor(Theme::getThemeColor(Theme::TEXT));

            if (!info.name.empty())
            {
                graphics->setFont(boldFont);
                graphics->drawText(info.name, 2, y);
            }

            graphics->setFont(getFont());

            int top = y + height / 2;

            graphics->drawText(model->getElementAt(i), 2, top);

            if (info.version.first > 0)
            {
                graphics->setColor(unsupported);

                graphics->drawText(info.version.second,
                                   getWidth() - info.version.first - 2, top);
            }
        }
    }

    unsigned int getRowHeight() const
    {
        return 2 * getFont()->getHeight();
    }
};


ServerDialog::ServerDialog(ServerInfo *serverInfo, const std::string &dir):
    Window(_("Choose Your Server")),
    mDir(dir),
    mDownloadStatus(DOWNLOADING_PREPARING),
    mDownloadProgress(-1.0f),
    mServers(ServerInfos()),
    mServerInfo(serverInfo)
{
    setWindowName("ServerDialog");

    loadCustomServers();

    mServersListModel = new ServersListModel(&mServers, this);

    mServersList = new ServersListBox(mServersListModel);

    ScrollArea *usedScroll = new ScrollArea(mServersList);
    usedScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mDescription = new Label(std::string());
    mDownloadText = new Label(std::string());

    mQuitButton = new Button(_("Quit"), "quit", this);
    mConnectButton = new Button(_("Connect"), "connect", this);
    mManualEntryButton = new Button(_("Add custom Server..."), "addEntry", this);
    mModifyButton = new Button(_("Modify..."), "modify", this);
    mDeleteButton = new Button(_("Delete"), "remove", this);

    mServersList->setActionEventId("connect");
    mServersList->addSelectionListener(this);
    usedScroll->setVerticalScrollAmount(0);

    place(0, 0, usedScroll, 6, 5).setPadding(3);
    place(0, 5, mDescription, 5);
    place(0, 6, mDownloadText, 5);
    place(0, 7, mManualEntryButton);
    place(1, 7, mModifyButton);
    place(2, 7, mDeleteButton);
    place(4, 7, mQuitButton);
    place(5, 7, mConnectButton);

    // Make sure the list has enough height
    getLayout().setRowHeight(3, 80);

    // Do this manually instead of calling reflowLayout so we can enforce a
    // minimum width.
    int width = 0, height = 0;
    getLayout().reflow(width, height);
    if (width < 400)
    {
        width = 400;
        getLayout().reflow(width, height);
    }

    setContentSize(width, height);

    setMinWidth(getWidth());
    setMinHeight(getHeight());
    setDefaultSize(getWidth(), getHeight(), ImageRect::CENTER);

    setResizable(true);
    addKeyListener(this);

    loadWindowState();

    mServersList->setSelected(0); // Do this after for the Delete button

    setVisible(true);

    mServersList->requestFocus();

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
}

void ServerDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        // Give focus back to the server dialog.
        mServersList->requestFocus();
    }
    else if (event.getId() == "connect")
    {
        int index = mServersList->getSelected();

        // Check login
        if (index < 0
#ifndef MANASERV_SUPPORT
            || mServersListModel->getServer(index).type == ServerInfo::MANASERV
#endif
        )
        {
            OkDialog *dlg = new OkDialog(_("Error"),
                _("Please select a valid server."));
            dlg->addActionListener(this);
        }
        else
        {
            mDownload->cancel();
            mQuitButton->setEnabled(false);
            mConnectButton->setEnabled(false);
            mDeleteButton->setEnabled(false);
            mManualEntryButton->setEnabled(false);
            mModifyButton->setEnabled(false);

            const ServerInfo &serverInfo = mServersListModel->getServer(index);
            mServerInfo->hostname = serverInfo.hostname;
            mServerInfo->name = serverInfo.name;
            mServerInfo->port = serverInfo.port;
            mServerInfo->type = serverInfo.type;

            // Save the selected server
            mServerInfo->save = true;
            saveCustomServers(*mServerInfo);

            chatLogger->setServerName(mServerInfo->hostname);

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
        // Add a custom server: It will delete itself using guichan logic.
        new CustomServerDialog(this);
    }
    else if (event.getId() == "modify")
    {
        int index = mServersList->getSelected();
        // Check whether a server is selected.
        if (index < 0)
        {
            OkDialog *dlg = new OkDialog(_("Error"),
                _("Please select a custom server."));
            dlg->addActionListener(this);
        }
        else
        {
            new CustomServerDialog(this, index);
        }
    }
    else if (event.getId() == "remove")
    {
        int index = mServersList->getSelected();
        mServers.erase(mServers.begin() + index);
        mServersList->setSelected(0);

        saveCustomServers();
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
        mModifyButton->setEnabled(false);
        return;
    }

    // Update the server and post fields according to the new selection
    const ServerInfo &myServer = mServersListModel->getServer(index);
    mDescription->setCaption(myServer.description);

    mDeleteButton->setEnabled(myServer.save);
    mModifyButton->setEnabled(myServer.save);
}

void ServerDialog::mouseClicked(gcn::MouseEvent &mouseEvent)
{
    if (mouseEvent.getSource() == mServersList &&
        isDoubleClick(mServersList->getSelected()))
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

            mDescription->setCaption(mServers[0].description);
            mDownloadText->setCaption(std::string());
        }
        else if (mDownloadStatus == DOWNLOADING_IN_PROGRESS)
        {
            mDownloadText->setCaption(strprintf(_("Downloading server list..."
                                                 "%2.2f%%"),
                                      mDownloadProgress * 100));
        }
        else if (mDownloadStatus == DOWNLOADING_IDLE)
        {
            mDownloadText->setCaption(_("Waiting for server..."));
        }
        else if (mDownloadStatus == DOWNLOADING_PREPARING)
        {
            mDownloadText->setCaption(_("Preparing download"));
        }
        else if (mDownloadStatus == DOWNLOADING_ERROR)
        {
            mDownloadText->setCaption(_("Error retreiving server list!"));
        }
    }

    Window::logic();
}

void ServerDialog::downloadServerList()
{
    // Try to load the configuration value for the onlineServerList
    std::string listFile = branding.getStringValue("onlineServerList");

    if (listFile.empty())
        listFile = config.getStringValue("onlineServerList");

    // Fall back to manasource.org when neither branding nor config set it
    if (listFile.empty())
        listFile = "http://www.manasource.org/serverlist.xml";

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

        server.type = ServerInfo::parseType(type);

        // Ignore unknown server types
        if (server.type == ServerInfo::UNKNOWN
#ifndef MANASERV_SUPPORT
            || server.type == ServerInfo::MANASERV
#endif
        )
        {
            logger->log("Ignoring server entry with unknown type: %s",
                        type.c_str());
            continue;
        }

        server.name = XML::getProperty(serverNode, "name", std::string());

        std::string version = XML::getProperty(serverNode, "minimumVersion",
                                               std::string());

        bool meetsMinimumVersion = (compareStrI(version, PACKAGE_VERSION)
                                      <= 0);

        // For display in the list
        if (meetsMinimumVersion)
            version.clear();
        else if (version.empty())
            version = _("requires a newer version");
        else
            version = strprintf(_("requires v%s"), version.c_str());

        for_each_xml_child_node(subNode, serverNode)
        {
            if (xmlStrEqual(subNode->name, BAD_CAST "connection"))
            {
                server.hostname = XML::getProperty(subNode, "hostname", "");
                server.port = XML::getProperty(subNode, "port", 0);
                if (server.port == 0)
                {
                    // If no port is given, use the default for the given type
                    server.port = ServerInfo::defaultPortForServerType(server.type);
                }
            }
            else if (xmlStrEqual(subNode->name, BAD_CAST "description"))
            {
                server.description = (const char*) subNode->xmlChildrenNode->content;
            }
        }

        server.version.first = gui->getFont()->getWidth(version);
        server.version.second = version;

        MutexLocker lock(&mMutex);
        // Add the server to the local list if it's not already present
        bool found = false;
        int i = 0;
        for (ServerInfos::iterator it = mServers.begin(); it != mServers.end();
             ++it)
        {
            if (*it == server)
            {
                // Use the name listed in the server list
                (*it).name = server.name;
                (*it).version = server.version;
                (*it).description = server.description;
                mServersListModel->setVersionString(i, version);
                found = true;
                break;
            }
            ++i;
        }

        if (!found)
            mServers.push_back(server);
    }
}

void ServerDialog::loadCustomServers()
{
    for (int i = 0; i < MAX_SERVERLIST; ++i)
    {
        const std::string index = toString(i);
        const std::string nameKey = "MostUsedServerDescName" + index;
        const std::string hostNameKey = "MostUsedServerName" + index;
        const std::string typeKey = "MostUsedServerType" + index;
        const std::string portKey = "MostUsedServerPort" + index;
        const std::string descriptionKey = "MostUsedServerDescription" + index;

        ServerInfo server;
        server.name = config.getValue(nameKey, "");
        server.hostname = config.getValue(hostNameKey, "");
        server.type = ServerInfo::parseType(config.getValue(typeKey, ""));

        const int defaultPort = ServerInfo::defaultPortForServerType(server.type);
        server.port = (unsigned short) config.getValue(portKey, defaultPort);
        server.description = config.getValue(descriptionKey, "");

        // Stop on the first invalid server
        if (!server.isValid())
            break;

        server.save = true;

        mServers.push_back(server);
    }
}

void ServerDialog::saveCustomServers(const ServerInfo &currentServer, int index)
{
    ServerInfos::iterator it, it_end = mServers.end();

    // Make sure the current server is mentioned first
    if (currentServer.isValid())
    {
        if (index > -1)
        {
            mServers[index] = currentServer;
        }
        else
        {
            for (it = mServers.begin(); it != it_end; ++it)
            {
                if (*it == currentServer)
                {
                    mServers.erase(it);
                    break;
                }
            }
            mServers.push_front(currentServer);
        }
    }

    int savedServerCount = 0;

    for (it = mServers.begin(), it_end = mServers.end();
         it != it_end && savedServerCount < MAX_SERVERLIST; ++it)
    {
        const ServerInfo &server = *it;

        // Only save servers that were loaded from settings
        if (!(server.save && server.isValid()))
            continue;

        const std::string index = toString(savedServerCount);
        const std::string nameKey = "MostUsedServerDescName" + index;
        const std::string hostNameKey = "MostUsedServerName" + index;
        const std::string typeKey = "MostUsedServerType" + index;
        const std::string portKey = "MostUsedServerPort" + index;
        const std::string descriptionKey = "MostUsedServerDescription" + index;

        config.setValue(hostNameKey, toString(server.hostname));
        config.setValue(typeKey, serverTypeToString(server.type));
        config.setValue(portKey, toString(server.port));
        config.setValue(nameKey, server.name);
        config.setValue(descriptionKey, server.description);
        ++savedServerCount;
    }

    // Insert an invalid entry at the end to make the loading stop there
    if (savedServerCount < MAX_SERVERLIST)
        config.setValue("MostUsedServerName" + toString(savedServerCount), "");

    // Restore the correct description
    if (index < 0)
        index = 0;
    mDescription->setCaption(mServers[index].description);
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
        logger->log("Error retreiving server list: %s",
                    sd->mDownload->getError());
        sd->mDownloadStatus = DOWNLOADING_ERROR;
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
