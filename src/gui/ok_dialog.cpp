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

#include "ok_dialog.h"

OkDialog::OkDialog(gcn::Container *parent, std::string msg):
    Window(parent, "Message")
{
    userLabel = new gcn::Label(msg);
    okButton = new Button("OK");

    setSize(200, 80);
    userLabel->setPosition(4, 11);
    okButton->setPosition(120, 52);
    okButton->setEventId("ok");

    okButton->addActionListener(this);

    add(userLabel);
    add(okButton);

    setLocationRelativeTo(getParent());
    okButton->requestFocus();
}

OkDialog::~OkDialog()
{
    delete userLabel;
    delete okButton;
}

void OkDialog::action(const std::string& eventId)
{
    if (eventId == "ok") {
    }
}
