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
#ifdef MANASERV_SUPPORT
    if (elementIndex == 1)
        return "ManaServ";
#endif
    return "Unknown";
}

CustomServerDialog::CustomServerDialog(ServerDialog *parent, int index):
    Window(_("Custom Server"), true, parent),
    mServerDialog(parent),
    mIndex(index)
{
    setWindowName("CustomServerDialog");

    auto *nameLabel = new Label(_("Name:"));
    auto *serverAdressLabel = new Label(_("Address:"));
    auto *portLabel = new Label(_("Port:"));
#ifdef MANASERV_SUPPORT
    auto *typeLabel = new Label(_("Server type:"));
#endif
    auto *descriptionLabel = new Label(_("Description:"));
    mServerAddressField = new TextField(std::string());
    mPortField = new TextField(std::string());

#ifdef MANASERV_SUPPORT
    mTypeListModel = std::make_unique<TypeListModel>();
    mTypeField = new DropDown(mTypeListModel.get());
    mTypeField->setSelected(0); // TmwAthena by default for now.
#endif

    mNameField = new TextField(std::string());
    mDescriptionField = new TextField(std::string());

    mOkButton = new Button(_("Ok"), "addServer", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    mServerAddressField->addActionListener(this);
    mPortField->addActionListener(this);

    place(0, 0, nameLabel);
    place(1, 0, mNameField, 4).setPadding(2);
    place(0, 1, serverAdressLabel);
    place(1, 1, mServerAddressField, 4).setPadding(2);
    place(0, 2, portLabel);
    place(1, 2, mPortField, 4).setPadding(2);
#ifdef MANASERV_SUPPORT
    place(0, 3, typeLabel);
    place(1, 3, mTypeField).setPadding(2);
#endif
    place(0, 4, descriptionLabel);
    place(1, 4, mDescriptionField, 4).setPadding(2);
    place(4, 5, mOkButton);
    place(3, 5, mCancelButton);

    reflowLayout();
    setLocationRelativeTo(getParentWindow());

    addKeyListener(this);

    // Add the entry's info when in modify mode.
    if (index > -1)
    {
        const ServerInfo &serverInfo = mServerDialog->mServers[index];
        mNameField->setText(serverInfo.name);
        mDescriptionField->setText(serverInfo.description);
        mServerAddressField->setText(serverInfo.hostname);
        mPortField->setText(toString(serverInfo.port));
#ifdef MANASERV_SUPPORT
        mTypeField->setSelected(serverInfo.type == ServerType::TmwAthena ?
                                0 : 1);
#endif
    }

    setVisible(true);

    mNameField->requestFocus();
}

CustomServerDialog::~CustomServerDialog() = default;

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
            auto *dlg = new OkDialog(_("Error"),
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
#ifdef MANASERV_SUPPORT
            switch (mTypeField->getSelected())
            {
                case 0:
                    serverInfo.type = ServerType::TmwAthena;
                    break;
                case 1:
                    serverInfo.type = ServerType::ManaServ;
                    break;
                default:
                    serverInfo.type = ServerType::Unknown;
            }
#else
            serverInfo.type = ServerType::TMWATHENA;
#endif
            if (mPortField->getText().empty())
                serverInfo.port = ServerInfo::defaultPortForServerType(serverInfo.type);
            else
                serverInfo.port = static_cast<uint16_t>(atoi(mPortField->getText().c_str()));

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
        action(gcn::ActionEvent(nullptr, mOkButton->getActionEventId()));
    }
}
