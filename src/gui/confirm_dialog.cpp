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

#include "../utils/gettext.h"

ConfirmDialog::ConfirmDialog(const std::string &title, const std::string &msg,
        Window *parent):
    Window(title, true, parent)
{
    gcn::Label *textLabel = new gcn::Label(msg);
    gcn::Button *yesButton = new Button(_("Yes"), "yes", this);
    gcn::Button *noButton = new Button(_("No"), "no", this);

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

    add(textLabel);
    add(yesButton);
    add(noButton);

    if (getParent()) {
        setLocationRelativeTo(getParent());
        getParent()->moveToTop(this);
    }
    setVisible(true);
    yesButton->requestFocus();
}

void ConfirmDialog::action(const gcn::ActionEvent &event)
{
    // Proxy button events to our listeners
    ActionListenerIterator i;
    for (i = mActionListeners.begin(); i != mActionListeners.end(); ++i)
    {
        (*i)->action(event);
    }

    // Can we receive anything else anyway?
    if (event.getId() == "yes" || event.getId() == "no")
    {
        scheduleDelete();
    }
}
