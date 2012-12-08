/*
 *  The Mana Fire Client
 *  Copyright (C) 2011-2012  The Mana Developers
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

#include "gui/customserverdialog.h"

#include "configuration.h"

#include "gui/okdialog.h"
#include "gui/sdlinput.h"
#include "gui/serverdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/textfield.h"

#include "utils/gettext.h"

CustomServerDialog::CustomServerDialog(ServerDialog *parent, int index):
    Window(_("Custom Server"), true, parent),
    mServerDialog(parent),
    mIndex(index)
{
    setWindowName("CustomServerDialog");

    Label *nameLabel = new Label(_("Name:"));
    Label *serverAdressLabel = new Label(_("Address:"));
    Label *portLabel = new Label(_("Port:"));
    Label *descriptionLabel = new Label(_("Description:"));
    mServerAddressField = new TextField(std::string());
    mPortField = new TextField(std::string());

    mNameField = new TextField(std::string());
    mDescriptionField = new TextField(std::string());

    mOkButton = new Button(_("Ok"), "addServer", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    mServerAddressField->addActionListener(this);
    mPortField->addActionListener(this);

    place(0, 0, nameLabel);
    place(1, 0, mNameField, 4).setPadding(3);
    place(0, 1, serverAdressLabel);
    place(1, 1, mServerAddressField, 4).setPadding(3);
    place(0, 2, portLabel);
    place(1, 2, mPortField, 4).setPadding(3);
    place(0, 4, descriptionLabel);
    place(1, 4, mDescriptionField, 4).setPadding(3);
    place(4, 5, mOkButton);
    place(3, 5, mCancelButton);

    // Do this manually instead of calling reflowLayout so we can enforce a
    // minimum width.
    int width = 0, height = 0;
    getLayout().reflow(width, height);
    if (width < 300)
    {
        width = 300;
        getLayout().reflow(width, height);
    }
    if (height < 120)
    {
        height = 120;
        getLayout().reflow(width, height);
    }

    setContentSize(width, height);

    setMinWidth(getWidth());
    setMinHeight(getHeight());
    setDefaultSize(getWidth(), getHeight(), ImageRect::CENTER);

    setResizable(false);
    addKeyListener(this);

    loadWindowState();

    // Add the entry's info when in modify mode.
    if (index > -1)
    {
        const ServerInfo &serverInfo = mServerDialog->mServers[index];
        mNameField->setText(serverInfo.name);
        mDescriptionField->setText(serverInfo.description);
        mServerAddressField->setText(serverInfo.hostname);
        mPortField->setText(toString(serverInfo.port));
    }

    setLocationRelativeTo(getParentWindow());
    setVisible(true);

    mNameField->requestFocus();
}

void CustomServerDialog::logic()
{
    Window::logic();
}

void CustomServerDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        // Give focus back to the server dialog.
        mServerAddressField->requestFocus();
    }
    if (event.getId() == "addServer")
    {
        // Check the given information
        if (mServerAddressField->getText().empty())
        {
            OkDialog *dlg = new OkDialog(_("Error"),
                _("Please type in at least the address of the server."));
            dlg->addActionListener(this);
        }
        else
        {
            mCancelButton->setEnabled(false);
            mOkButton->setEnabled(false);

            ServerInfo serverInfo;
            serverInfo.name = mNameField->getText();
            serverInfo.description = mDescriptionField->getText();
            serverInfo.hostname = mServerAddressField->getText();
            serverInfo.type = ServerInfo::MANASERV;
            if (mPortField->getText().empty())
                serverInfo.port = ServerInfo::defaultPortForServerType(serverInfo.type);
            else
                serverInfo.port = (short) atoi(mPortField->getText().c_str());

            // Tell the server has to be saved
            serverInfo.save = true;

            //Add server
            mServerDialog->saveCustomServers(serverInfo, mIndex);
            scheduleDelete();
        }
    }
    else if (event.getId() == "cancel")
    {
        scheduleDelete();
    }
}

void CustomServerDialog::keyPressed(gcn::KeyEvent &keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ESCAPE)
    {
        scheduleDelete();
    }
    else if (key.getValue() == Key::ENTER)
    {
        action(gcn::ActionEvent(NULL, mOkButton->getActionEventId()));
    }
}
