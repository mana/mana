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

#include "button.h"
#include "char_server.h"
#include "listbox.h"
#include "scrollarea.h"

#include "../logindata.h"
#include "../main.h"
#include "../serverinfo.h"

#include "../net/network.h" // TODO this is just for iptostring, move that?

#include "../utils/gettext.h"
#include "../utils/strprintf.h"
#include "../utils/tostring.h"

extern SERVER_INFO **server_info;

/**
 * The list model for the server list.
 */
class ServerListModel : public gcn::ListModel {
    public:
        virtual ~ServerListModel() {};

        int getNumberOfElements();
        std::string getElementAt(int i);
};

ServerSelectDialog::ServerSelectDialog(LoginData *loginData, int nextState):
    Window(_("Select Server")),
    mLoginData(loginData),
    mNextState(nextState)
{
    mServerListModel = new ServerListModel();
    mServerList = new ListBox(mServerListModel);
    ScrollArea *mScrollArea = new ScrollArea(mServerList);
    mOkButton = new Button(_("OK"), "ok", this);
    Button *mCancelButton = new Button(_("Cancel"), "cancel", this);

    setContentSize(200, 100);

    mCancelButton->setPosition(
            200 - mCancelButton->getWidth() - 5,
            100 - mCancelButton->getHeight() - 5);
    mOkButton->setPosition(
            mCancelButton->getX() - mOkButton->getWidth() - 5,
            100 - mOkButton->getHeight() - 5);
    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mScrollArea->setDimension(gcn::Rectangle(
                5, 5, 200 - 2 * 5,
                100 - 3 * 5 - mCancelButton->getHeight() -
                mScrollArea->getFrameSize()));

    mServerList->setActionEventId("ok");

    //mServerList->addActionListener(this);

    add(mScrollArea);
    add(mOkButton);
    add(mCancelButton);

    if (n_server == 0) {
        // Disable Ok button
        mOkButton->setEnabled(false);
    } else {
        // Select first server
        mServerList->setSelected(1);
    }

    setLocationRelativeTo(getParent());
    setVisible(true);
    mOkButton->requestFocus();
}

ServerSelectDialog::~ServerSelectDialog()
{
    delete mServerListModel;
}

void ServerSelectDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok") {
        mOkButton->setEnabled(false);
        const SERVER_INFO *si = server_info[mServerList->getSelected()];
        mLoginData->hostname = iptostring(si->address);
        mLoginData->port = si->port;
        mLoginData->updateHost = si->updateHost;
        state = mNextState;
    }
    else if (event.getId() == "cancel") {
        state = LOGIN_STATE;
    }
}

int ServerListModel::getNumberOfElements()
{
    return n_server;
}

std::string ServerListModel::getElementAt(int i)
{
    const SERVER_INFO *si = server_info[i];
    return si->name + " (" + toString(si->online_users) + ")";
}
