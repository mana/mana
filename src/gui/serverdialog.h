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

#include "guichanfwd.h"

#include "net/tmwserv/network.h"

#include <guichan/actionlistener.hpp>
#include <guichan/listmodel.hpp>

#include <string>
#include <vector>

class DropDown;
class LoginData;

/**
 * A server structure to keep pairs of servers and ports.
 */
struct Server
{
    Server():
        port(0)
    {}

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
        Server getServer(int elementIndex) const
        { return servers[elementIndex]; }

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
        void action(const gcn::ActionEvent &event);

    private:
        gcn::TextField *mServerNameField;
        gcn::TextField *mPortField;
        gcn::Button *mOkButton;
        gcn::Button *mCancelButton;

        DropDown *mMostUsedServersDropDown;
        ServersListModel *mMostUsedServersListModel;

        LoginData *mLoginData;
};

#endif
