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
        ServerInfo getServer(int elementIndex) const
        { return mServers->at(elementIndex); }

    private:
        ServerInfos *mServers;
        ServerDialog *mParent;
};

/**
 * Server and Port List Model
 */
class TypeListModel : public gcn::ListModel
{
    public:
        TypeListModel() { };

        /**
         * Used to get number of line in the list
         */
        int getNumberOfElements() { return 2; }

        /**
         * Used to get an element from the list
         */
        std::string getElementAt(int elementIndex);
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
        /**
         * Constructor
         *
         * @see Window::Window
         */
        ServerDialog(ServerInfo *serverInfo, const std::string &dir);

        /**
         * Destructor
         */
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

        void logic();

    protected:
        friend class ServersListModel;
        MutexLocker lock() { return MutexLocker(&mMutex); }

    private:
        /**
         * Called to load a list of available server from an online xml file.
         */
        void downloadServerList();
        void loadServers();
        static int downloadUpdate(void *ptr, DownloadStatus status,
                                  size_t total, size_t remaining);

        void setFieldsReadOnly(const bool readOnly);

        TextField *mServerNameField;
        TextField *mPortField;
        Label  *mDescription;
        Button *mQuitButton;
        Button *mConnectButton;
        Button *mManualEntryButton;

        ListBox *mServersList;
        ServersListModel *mServersListModel;

        DropDown *mTypeField;
        TypeListModel *mTypeListModel;

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

        Mutex mMutex;
        float mDownloadProgress;

        ServerInfos mServers;
        ServerInfo *mServerInfo;
};

#endif
