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

#include "resources/theme.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/xml.h"

#include <guichan/font.hpp>

#include <cstdlib>
#include <string>

ServersListModel::ServersListModel(ServerInfos *servers, ServerDialog *parent):
        mServers(servers),
        mVersionStrings(servers->size(), VersionString(0, std::string())),
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
        mVersionStrings[index] = VersionString(0, std::string());
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

    void draw(gcn::Graphics *graphics) override
    {
        if (!mListModel)
            return;

        auto *model = static_cast<ServersListModel*>(mListModel);

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

    unsigned int getRowHeight() const override
    {
        return 2 * getFont()->getHeight();
    }
};


ServerDialog::ServerDialog(ServerInfo *serverInfo, const std::string &dir):
    Window(_("Choose Your Server")),
    mDir(dir),
    mServerInfo(serverInfo)
{
    setWindowName("ServerDialog");

    loadCustomServers();

    mServersListModel = new ServersListModel(&mServers, this);

    mServersList = new ServersListBox(mServersListModel);

    auto *usedScroll = new ScrollArea(mServersList);
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

        // Make sure thread is gone before deleting the ServersListModel
        mDownload.reset();
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
            || mServersListModel->getServer(index).type == ServerType::MANASERV
#endif
        )
        {
            auto *dlg = new OkDialog(_("Error"),
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
            auto *dlg = new OkDialog(_("Error"),
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
        action(gcn::ActionEvent(nullptr, mConnectButton->getActionEventId()));
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
                                                 "%2.0f%%"),
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
        listFile = config.onlineServerList;

    // Fall back to manasource.org when neither branding nor config set it
    if (listFile.empty())
        listFile = "https://www.manasource.org/serverlist.xml";

    mDownload = std::make_unique<Net::Download>(this, listFile,
                                                &ServerDialog::downloadUpdate);
    mDownload->setFile(mDir + "/serverlist.xml");
    mDownload->start();
}

void ServerDialog::loadServers()
{
    XML::Document doc(mDir + "/serverlist.xml", false);
    XML::Node rootNode = doc.rootNode();

    if (!rootNode || rootNode.name() != "serverlist")
    {
        logger->log("Error loading server list!");
        return;
    }

    int version = rootNode.getProperty("version", 0);
    if (version != 1)
    {
        logger->log("Error: unsupported online server list version: %d",
                    version);
        return;
    }

    for (auto serverNode : rootNode.children())
    {
        if (serverNode.name() != "server")
            continue;

        ServerInfo server;

        std::string type = serverNode.getProperty("type", "unknown");

        server.type = ServerInfo::parseType(type);

        // Ignore unknown server types
        if (server.type == ServerType::UNKNOWN
#ifndef MANASERV_SUPPORT
            || server.type == ServerType::MANASERV
#endif
        )
        {
            logger->log("Ignoring server entry with unknown type: %s",
                        type.c_str());
            continue;
        }

        server.name = serverNode.getProperty("name", std::string());

        std::string version = serverNode.getProperty("minimumVersion",
                                               std::string());

        bool meetsMinimumVersion = compareStrI(version, PACKAGE_VERSION) <= 0;

        // For display in the list
        if (meetsMinimumVersion)
            version.clear();
        else if (version.empty())
            version = _("requires a newer version");
        else
            version = strprintf(_("requires v%s"), version.c_str());

        for (auto subNode : serverNode.children())
        {
            if (subNode.name() == "connection")
            {
                server.hostname = subNode.getProperty("hostname", std::string());
                server.port = subNode.getProperty("port", 0);
                if (server.port == 0)
                {
                    // If no port is given, use the default for the given type
                    server.port = ServerInfo::defaultPortForServerType(server.type);
                }
            }
            else if (subNode.name() == "description")
            {
                server.description = subNode.textContent();
            }
            else if (subNode.name() == "persistentIp")
            {
                const auto text = subNode.textContent();
                server.persistentIp = text == "1" || text == "true";
            }
        }

        server.version.first = gui->getFont()->getWidth(version);
        server.version.second = version;

        // Add the server to the local list if it's not already present
        bool found = false;
        int i = 0;
        for (auto &s : mServers)
        {
            if (s == server)
            {
                // Use the name listed in the server list
                s.name = server.name;
                s.version = server.version;
                s.description = server.description;
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
    for (auto &server : config.servers)
    {
        if (server.isValid())
        {
            server.save = true;
            mServers.push_back(server);
        }
    }
}

void ServerDialog::saveCustomServers(const ServerInfo &currentServer, int index)
{
    // Make sure the current server is mentioned first
    if (currentServer.isValid())
    {
        if (index > -1)
        {
            mServers[index] = currentServer;
        }
        else
        {
            for (auto it = mServers.begin(), it_end = mServers.end(); it != it_end; ++it)
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

    config.servers = mServers;

    // Restore the correct description
    if (index < 0)
        index = 0;
    mDescription->setCaption(mServers[index].description);
}

int ServerDialog::downloadUpdate(void *ptr, DownloadStatus status,
                                 size_t dltotal, size_t dlnow)
{
    if (status == DOWNLOAD_STATUS_CANCELLED)
        return -1;

    auto *sd = reinterpret_cast<ServerDialog*>(ptr);
    MutexLocker lock(&sd->mMutex);

    if (status == DOWNLOAD_STATUS_COMPLETE)
    {
        sd->loadServers();
        sd->mDownloadStatus = DOWNLOADING_COMPLETE;
    }
    else if (status < 0)
    {
        logger->log("Error retreiving server list: %s",
                    sd->mDownload->getError());
        sd->mDownloadStatus = DOWNLOADING_ERROR;
    }
    else
    {
        float progress = 0.0f;
        if (dltotal > 0)
            progress = static_cast<float>(dlnow) / dltotal;

        sd->mDownloadStatus = DOWNLOADING_IN_PROGRESS;
        sd->mDownloadProgress = progress;
    }

    return 0;
}
