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

#include "confirm_dialog.h"

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "windowcontainer.h"


ConfirmDialog::ConfirmDialog(const std::string &title, const std::string &msg,
        gcn::ActionListener *listener):
    Window(title, true)
{
    init(msg, listener);
}

ConfirmDialog::ConfirmDialog(Window *parent, const std::string &title,
        const std::string &msg, gcn::ActionListener *listener):
    Window(title, true, parent)
{
    init(msg, listener);
}

void ConfirmDialog::init(const std::string &msg, gcn::ActionListener *listener)
{
    gcn::Label *userLabel = new gcn::Label(msg);
    Button *yesButton = new Button("Yes");
    Button *noButton = new Button("No");

    int w = userLabel->getWidth() + 20;
    int inWidth = yesButton->getWidth() + noButton->getWidth() + 5;
    int h = userLabel->getHeight() + 25 + yesButton->getHeight();

    if (w < inWidth + 10) {
        w = inWidth + 10;
    }

    setContentSize(w, h);
    userLabel->setPosition(10, 10);
    yesButton->setPosition(
            (w - inWidth) / 2,
            h - 5 - noButton->getHeight());
    noButton->setPosition(
            yesButton->getX() + yesButton->getWidth() + 5,
            h - 5 - noButton->getHeight());

    yesButton->setEventId("yes");
    noButton->setEventId("no");
    yesButton->addActionListener(this);
    noButton->addActionListener(this);
    if (listener) {
        yesButton->addActionListener(listener);
        noButton->addActionListener(listener);
    }

    add(userLabel);
    add(yesButton);
    add(noButton);

    if (getParent()) {
        setLocationRelativeTo(getParent());
        getParent()->moveToTop(this);
    }
    yesButton->requestFocus();
}

void ConfirmDialog::action(const std::string &eventId)
{
    if (eventId == "yes" || eventId == "no") {
        windowContainer->scheduleDelete(this);
    }
}
