/*
 *  The Mana Fire Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *  Copyright (C) 2012-2012  The Land of Fire Developers
 *
 *  This file is part of The Mana Fire Client.
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

#ifndef SERVERDIALOG_H
#define SERVERDIALOG_H

#include "gui/widgets/window.h"

#include "net/download.h"
#include "net/serverinfo.h"

#include "utils/mutex.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/listmodel.hpp>
#include <guichan/selectionlistener.hpp>

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
        typedef std::pair<int, std::string> VersionString;

        ServersListModel(ServerInfos *servers, ServerDialog *parent);

        /**
         * Used to get number of line in the list
         */
        int getNumberOfElements();

        /**
         * Used to get an element from the list
         */
        std::string getElementAt(int elementIndex);

        /**
         * Used to get the corresponding Server struct
         */
        const ServerInfo &getServer(int elementIndex) const
        { return mServers->at(elementIndex); }

        void setVersionString(int index, const std::string &version);

    private:
        typedef std::vector<VersionString> VersionStrings;

        ServerInfos *mServers;
        VersionStrings mVersionStrings;
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

        ~ServerDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        void keyPressed(gcn::KeyEvent &keyEvent);

        /**
         * Called when the selected value changed in the servers list box.
         */
        void valueChanged(const gcn::SelectionEvent &event);

        void mouseClicked(gcn::MouseEvent &mouseEvent);

        void logic();

    protected:
        friend class ServersListModel;
        Mutex *getMutex() { return &mMutex; }

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

        void loadCustomServers();

        static int downloadUpdate(void *ptr, DownloadStatus status,
                                  size_t total, size_t remaining);

        Label  *mDescription;
        Button *mQuitButton;
        Button *mConnectButton;
        Button *mManualEntryButton;
        Button *mModifyButton;
        Button *mDeleteButton;

        ListBox *mServersList;
        ServersListModel *mServersListModel;

        const std::string &mDir;

        enum ServerDialogDownloadStatus
        {
            DOWNLOADING_ERROR,
            DOWNLOADING_PREPARING,
            DOWNLOADING_IDLE,
            DOWNLOADING_IN_PROGRESS,
            DOWNLOADING_COMPLETE,
            DOWNLOADING_OVER
        };

        /** Status of the current download. */
        ServerDialogDownloadStatus mDownloadStatus;

        Net::Download *mDownload;
        Label  *mDownloadText;

        Mutex mMutex;
        float mDownloadProgress;

        ServerInfos mServers;
        ServerInfo *mServerInfo;
};

#endif
