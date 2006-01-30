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


ConfirmDialog::ConfirmDialog(const std::string &title, const std::string &msg,
        gcn::ActionListener *listener, Window *parent):
    Window(title, true, parent)
{
    gcn::Label *textLabel = new gcn::Label(msg);
    gcn::Button *yesButton = new Button("Yes");
    gcn::Button *noButton = new Button("No");

    int w = textLabel->getWidth() + 20;
    int inWidth = yesButton->getWidth() + noButton->getWidth() + 5;
    int h = textLabel->getHeight() + 25 + yesButton->getHeight();

    if (w < inWidth + 10) {
        w = inWidth + 10;
    }

    setContentSize(w, h);
    textLabel->setPosition(10, 10);
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

    add(textLabel);
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
    // Can we receive anything else anyway?
    if (eventId == "yes" || eventId == "no") {
        scheduleDelete();
    }
}
