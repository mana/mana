/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gui/textdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/textfield.h"

#include "utils/gettext.h"

TextDialog::TextDialog(const std::string &title, const std::string &msg,
                       Window *parent):
    Window(title, true, parent),
    mTextField(new TextField)
{
    gcn::Label *textLabel = new Label(msg);
    mOkButton = new Button(_("OK"), "OK", this);
    gcn::Button *cancelButton = new Button(_("Cancel"), "CANCEL", this);

    int w = textLabel->getWidth() + 20;
    int inWidth = mOkButton->getWidth() + cancelButton->getWidth() + 5;
    int h = textLabel->getHeight() + 25 + mOkButton->getHeight() + mTextField->getHeight();

    if (w < inWidth + 10)
        w = inWidth + 10;

    setContentSize(w, h);
    textLabel->setPosition(10, 10);
    mTextField->setWidth(85);
    mTextField->setPosition(10,20 + textLabel->getHeight());
    mOkButton->setPosition((w - inWidth) / 2,
                           h - 5 - cancelButton->getHeight());
    cancelButton->setPosition(mOkButton->getX() + mOkButton->getWidth() + 5,
                          h - 5 - cancelButton->getHeight());

    add(textLabel);
    add(mTextField);
    add(mOkButton);
    add(cancelButton);

    if (getParent()) {
        setLocationRelativeTo(getParent());
        getParent()->moveToTop(this);
    }
    setVisible(true);
    mTextField->requestFocus();
}

void TextDialog::action(const gcn::ActionEvent &event)
{
    // Proxy button events to our listeners
    ActionListenerIterator i;
    for (i = mActionListeners.begin(); i != mActionListeners.end(); ++i)
    {
        (*i)->action(event);
    }

    if (event.getId() == "CANCEL" || event.getId() == "OK")
    {
        scheduleDelete();
    }
}

const std::string &TextDialog::getText() const
{
    return mTextField->getText();
}

void TextDialog::setOKButtonActionId(const std::string &name)
{
    mOkButton->setActionEventId(name);
}
