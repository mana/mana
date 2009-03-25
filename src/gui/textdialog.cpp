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

#include "textdialog.h"

#include <guichan/widgets/label.hpp>
#include <guichan/widgets/textfield.hpp>

#include "button.h"

TextDialog::TextDialog(const std::string &title, const std::string &msg,
                             Window *parent):
    Window(title, true, parent),
    textField(new TextField(""))
{
    gcn::Label *textLabel = new gcn::Label(msg);
    okButton = new Button("OK", "OK", this);
    gcn::Button *cancelButton = new Button("Cancel", "CANCEL", this);

    int w = textLabel->getWidth() + 20;
    int inWidth = okButton->getWidth() + cancelButton->getWidth() + 5;
    int h = textLabel->getHeight() + 25 + okButton->getHeight() + textField->getHeight();

    if (w < inWidth + 10) {
        w = inWidth + 10;
    }

    setContentSize(w, h);
    textLabel->setPosition(10, 10);
    textField->setWidth(85);
    textField->setPosition(10,20 + textLabel->getHeight());
    okButton->setPosition((w - inWidth) / 2,
                           h - 5 - cancelButton->getHeight());
    cancelButton->setPosition(okButton->getX() + okButton->getWidth() + 5,
                          h - 5 - cancelButton->getHeight());

    add(textLabel);
    add(textField);
    add(okButton);
    add(cancelButton);

    if (getParent()) {
        setLocationRelativeTo(getParent());
        getParent()->moveToTop(this);
    }
    setVisible(true);
    textField->requestFocus();
}

void TextDialog::action(const gcn::ActionEvent &event)
{
    // Proxy button events to our listeners
    ActionListenerIterator i;
    for (i = mActionListeners.begin(); i != mActionListeners.end(); ++i)
    {
        (*i)->action(event);
    }

    if(event.getId() == "CANCEL" || event.getId() == "OK")
    {
        scheduleDelete();
    }
}

const std::string &TextDialog::getText() const
{
    return textField->getText();
}

void TextDialog::setOKButtonActionId(const std::string &name)
{
    okButton->setActionEventId(name);
}
