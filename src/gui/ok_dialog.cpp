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
#include "button.h"

OkDialog::OkDialog(const std::string &title, const std::string &msg):
    Window(title, true)
{
    init(msg);
}

OkDialog::OkDialog(Window *parent, const std::string &title,
        const std::string &msg):
    Window(title, true, parent)
{
    init(msg);
}

void OkDialog::init(const std::string &msg)
{
    userLabel = new gcn::Label(msg);
    okButton = new Button("OK");

    int w = userLabel->getWidth() + 20;
    int h = userLabel->getHeight() + 25 + okButton->getHeight();

    if (okButton->getWidth() + 10 > w) {
        w = okButton->getWidth() + 10;
    }

    setSize(w, h);
    userLabel->setPosition(10, 10);
    okButton->setPosition(
            (w - okButton->getWidth()) / 2,
            h - 5 - okButton->getHeight());

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

void OkDialog::action(const std::string &eventId)
{
    if (eventId == "ok") {
        windowContainer->scheduleDelete(this);
    }
}
