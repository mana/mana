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

#pragma once

#include "gui/widgets/window.h"

#include "net/download.h"
#include "net/serverinfo.h"
#include "utils/xml.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/listmodel.hpp>
#include <guichan/selectionlistener.hpp>

#include <memory>
#include <string>
#include <vector>

class Button;
class Label;
class ListBox;
class ServerDialog;
class TextField;
class DropDown;

/**
 * Server and Port List Model
 */
class ServersListModel : public gcn::ListModel
{
    public:
        using VersionString = std::pair<int, std::string>;

        ServersListModel(ServerInfos *servers, ServerDialog *parent);

        /**
         * Used to get number of line in the list
         */
        int getNumberOfElements() override;

        /**
         * Used to get an element from the list
         */
        std::string getElementAt(int elementIndex) override;

        /**
         * Used to get the corresponding Server struct
         */
        const ServerInfo &getServer(int elementIndex) const
        { return mServers->at(elementIndex); }

        void setVersionString(int index, const std::string &version);

    private:
        ServerInfos *mServers;
        std::vector<VersionString> mVersionStrings;
        ServerDialog *mParent;
};

/**
 * The server choice dialog.
 *
 * \ingroup Interface
 */
class ServerDialog : public Window,
                     public gcn::ActionListener,
                     public gcn::KeyListener,
                     public gcn::SelectionListener
{
    public:
        ServerDialog(ServerInfo *serverInfo, const std::string &dir);
        ~ServerDialog() override;

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event) override;

        void keyPressed(gcn::KeyEvent &keyEvent) override;

        /**
         * Called when the selected value changed in the servers list box.
         */
        void valueChanged(const gcn::SelectionEvent &event) override;

        void mouseClicked(gcn::MouseEvent &mouseEvent) override;

        void logic() override;

    protected:
        friend class CustomServerDialog;

        /**
         * Saves the new server entry in the custom server list.
         * Removes the given entry when the serverInfo is empty.
         * Modifies the server entry given at index when it's not -1.
         */
        void saveCustomServers(const ServerInfo &currentServer = ServerInfo(),
                               int index = -1);

    private:
        /**
         * Called to load a list of available server from an online xml file.
         */
        void downloadServerList();
        void loadServers();
        void loadServer(XML::Node serverNode);

        void loadCustomServers();

        Label  *mDescription;
        Button *mQuitButton;
        Button *mConnectButton;
        Button *mManualEntryButton;
        Button *mModifyButton;
        Button *mDeleteButton;

        ListBox *mServersList;
        std::unique_ptr<ServersListModel> mServersListModel;

        const std::string &mDir;

        std::unique_ptr<Net::Download> mDownload;
        bool mDownloadDone = false;
        Label *mDownloadText;
        ServerInfos mServers;
        ServerInfo *mServerInfo;
};
