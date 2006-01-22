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
 *  $Id$
 */

#include "char_server.h"

#include <sstream>

#include "button.h"
#include "listbox.h"
#include "scrollarea.h"

#include "../main.h"
#include "../serverinfo.h"

extern SERVER_INFO **server_info;

ServerSelectDialog::ServerSelectDialog():
    Window("Select Server")
{
    serverListModel = new ServerListModel();
    serverList = new ListBox(serverListModel);
    scrollArea = new ScrollArea(serverList);
    okButton = new Button("OK");
    cancelButton = new Button("Cancel");

    setContentSize(200, 100);

    cancelButton->setPosition(
            200 - cancelButton->getWidth() - 5,
            100 - cancelButton->getHeight() - 5);
    okButton->setPosition(
            cancelButton->getX() - okButton->getWidth() - 5,
            100 - okButton->getHeight() - 5);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setDimension(gcn::Rectangle(
                5, 5, 200 - 2 * 5,
                100 - 3 * 5 - cancelButton->getHeight() -
                scrollArea->getBorderSize()));

    serverList->setEventId("ok");
    okButton->setEventId("ok");
    cancelButton->setEventId("cancel");

    //serverList->addActionListener(this);
    okButton->addActionListener(this);
    cancelButton->addActionListener(this);

    add(scrollArea);
    add(okButton);
    add(cancelButton);

    if (n_server == 0) {
        // Disable Ok button
        okButton->setEnabled(false);
    } else {
        // Select first server
        serverList->setSelected(1);
    }

    okButton->requestFocus();
    setLocationRelativeTo(getParent());
}

ServerSelectDialog::~ServerSelectDialog()
{
    delete serverListModel;
}

void
ServerSelectDialog::action(const std::string& eventId)
{
    if (eventId == "ok") {
        okButton->setEnabled(false);
        state = CHAR_CONNECT_STATE;
    }
    else if (eventId == "cancel") {
        state = LOGIN_STATE;
    }
}

SERVER_INFO* ServerSelectDialog::getServerInfo()
{
    return server_info[serverList->getSelected()];
}

int
ServerListModel::getNumberOfElements()
{
    return n_server;
}

std::string
ServerListModel::getElementAt(int i)
{
    std::stringstream s;
    s << server_info[i]->name << " (" << server_info[i]->online_users << ")";
    return s.str();
}
