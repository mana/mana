/*
 *  The Mana Client
 *  Copyright (C) 2011-2012  The Mana Developers
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

#include "gui/customserverdialog.h"

#include "configuration.h"

#include "gui/okdialog.h"
#include "gui/sdlinput.h"
#include "gui/serverdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/dropdown.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/textfield.h"

#include "utils/gettext.h"

std::string TypeListModel::getElementAt(int elementIndex)
{
    if (elementIndex == 0)
        return "TmwAthena";
#ifdef ENABLE_MANASERV
    else if (elementIndex == 1)
        return "ManaServ";
#endif
    else
        return "Unknown";
}

CustomServerDialog::CustomServerDialog(ServerDialog *parent, int index):
    Window(_("Custom Server"), true, parent),
    mServerDialog(parent),
    mIndex(index)
{
    setWindowName("CustomServerDialog");

    Label *nameLabel = new Label(_("Name:"));
    Label *serverAdressLabel = new Label(_("Address:"));
    Label *portLabel = new Label(_("Port:"));
#ifdef ENABLE_MANASERV
    Label *typeLabel = new Label(_("Server type:"));
#endif
    Label *descriptionLabel = new Label(_("Description:"));
    mServerAddressField = new TextField(std::string());
    mPortField = new TextField(std::string());

#ifdef ENABLE_MANASERV
    mTypeListModel = new TypeListModel();
    mTypeField = new DropDown(mTypeListModel);
    mTypeField->setSelected(0); // TmwAthena by default for now.
#endif

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
#ifdef ENABLE_MANASERV
    place(0, 3, typeLabel);
    place(1, 3, mTypeField).setPadding(3);
#endif
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
#ifdef ENABLE_MANASERV
        mTypeField->setSelected(serverInfo.type == ServerInfo::TMWATHENA ?
                                0 : 1);
#endif
    }

    setLocationRelativeTo(getParentWindow());
    setVisible(true);

    mNameField->requestFocus();
}

CustomServerDialog::~CustomServerDialog()
{
#ifdef ENABLE_MANASERV
    delete mTypeListModel;
#endif
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
        if (mServerAddressField->getText().empty()
            || mPortField->getText().empty())
        {
            OkDialog *dlg = new OkDialog(_("Error"),
                _("Please at least type both the address and the port "
                  "of the server."));
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
            serverInfo.port = (short) atoi(mPortField->getText().c_str());
#ifdef ENABLE_MANASERV
            switch (mTypeField->getSelected())
            {
                case 0:
                    serverInfo.type = ServerInfo::TMWATHENA;
                    break;
                case 1:
                    serverInfo.type = ServerInfo::MANASERV;
                    break;
                default:
                    serverInfo.type = ServerInfo::UNKNOWN;
            }
#else
            serverInfo.type = ServerInfo::TMWATHENA;
#endif

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
