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
 */

#include "ok_dialog.h"

#include <guichan/widgets/label.hpp>

#include "button.h"

#include "../utils/gettext.h"

OkDialog::OkDialog(const std::string &title, const std::string &msg,
        Window *parent):
    Window(title, true, parent)
{
    gcn::Label *textLabel = new gcn::Label(msg);
    gcn::Button *okButton = new Button(_("Ok"), "ok", this);

    int w = textLabel->getWidth() + 20;
    int h = textLabel->getHeight() + 25 + okButton->getHeight();

    if (okButton->getWidth() + 10 > w) {
        w = okButton->getWidth() + 10;
    }

    setContentSize(w, h);
    textLabel->setPosition(10, 10);
    okButton->setPosition((w - okButton->getWidth()) / 2,
                          h - 5 - okButton->getHeight());

    add(textLabel);
    add(okButton);

    setLocationRelativeTo(getParent());
    setVisible(true);
    okButton->requestFocus();
}

void OkDialog::action(const gcn::ActionEvent &event)
{
    // Proxy button events to our listeners
    ActionListenerIterator i;
    for (i = mActionListeners.begin(); i != mActionListeners.end(); ++i)
    {
        (*i)->action(event);
    }

    // Can we receive anything else anyway?
    if (event.getId() == "ok") {
        scheduleDelete();
    }
}
