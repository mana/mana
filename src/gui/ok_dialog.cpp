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
#include "textbox.h"
#include "button.h"
#include "scrollarea.h"

OkDialog::OkDialog(const std::string &title, const std::string &msg,
        Window *parent):
    Window(title, true, parent)
{
    TextBox *textBox = new TextBox();
    textBox->setEditable(false);

    gcn::ScrollArea *scrollArea = new ScrollArea(textBox);
    gcn::Button *okButton = new Button("Ok", "ok", this);

    setContentSize(260, 175);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);
    scrollArea->setDimension(gcn::Rectangle(
                5, 5, 250, 160 - okButton->getHeight()));

    textBox->setTextWrapped(msg);

    okButton->setPosition(
            260 - 5 - okButton->getWidth(),
            175 - 5 - okButton->getHeight());

    add(scrollArea);
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
