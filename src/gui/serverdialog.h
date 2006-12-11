/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id: login.h 2486 2006-07-30 14:33:28Z b_lindeijer $
 */

#ifndef _TMW_SERVERDIALOG_H
#define _TMW_SERVERDIALOG_H

#include <iosfwd>
#include <vector>

#include <guichan/actionlistener.hpp>
#include <guichan/listmodel.hpp>
#include "./widgets/dropdown.h"

#include "login.h"
#include "window.h"

#include "../guichanfwd.h"

#include "../net/network.h"

class LoginData;

/**
 * A server structure to keep pairs of servers and ports.
 */
struct Server {
    Server():
        serverName(""),
        port(0) {};

    std::string serverName;
    short port;
};

/**
 * Server and Port List Model
 */
class ServersListModel : public gcn::ListModel
{
    public:
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
        Server getServer(int elementIndex) { return servers[elementIndex]; };
        /**
         * Add an Element at the end of the server list
         */
        void addElement(Server server);
        /**
         * Add an Element at the beginning of the server list
         */
        void addFirstElement(Server server);

    private:
        std::vector<Server> servers;
};

/**
 * Listener used for handling the DropDown in the server Dialog.
 */
class DropDownListener : public gcn::ActionListener
{
    public:
        DropDownListener(gcn::TextField *serverNameField,
                         gcn::TextField *serverPortField,
                         ServersListModel *serversListModel,
                         gcn::ListBox *serversListBox):
            currentSelectedIndex(0),
            mServerNameField(serverNameField),
            mServerPortField(serverPortField),
            mServersListModel(serversListModel),
            mServersListBox(serversListBox) {};
        void action(const std::string& eventId,
                    gcn::Widget* widget);
    private:
        short currentSelectedIndex;
        gcn::TextField *mServerNameField;
        gcn::TextField *mServerPortField;
        ServersListModel *mServersListModel;
        gcn::ListBox *mServersListBox;
};


/**
 * The server choice dialog.
 *
 * \ingroup Interface
 */
class ServerDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor
         *
         * @see Window::Window
         */
        ServerDialog(LoginData *loginData);

        /**
         * Destructor
         */
        ~ServerDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string &eventId, gcn::Widget *widget);

    private:
        gcn::TextField *mServerNameField;
        gcn::TextField *mPortField;
        gcn::Button *mOkButton;
        gcn::Button *mCancelButton;

        DropDown *mMostUsedServersDropDown;
        gcn::ListBox *mMostUsedServersListBox;
        gcn::ScrollArea *mMostUsedServersScrollArea;
        ServersListModel *mMostUsedServersListModel;

        DropDownListener *mDropDownListener;

        LoginData *mLoginData;
};

#endif
