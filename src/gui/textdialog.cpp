/*
 *  The Mana Client
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#include "gui/textdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/textfield.h"

#include "utils/gettext.h"

int TextDialog::instances = 0;

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

    if (getParent())
    {
        setLocationRelativeTo(getParent());
        getParent()->moveToTop(this);
    }
    setVisible(true);
    requestModalFocus();
    mTextField->requestFocus();
    
    instances++;
}

TextDialog::~TextDialog()
{
    instances--;
}

void TextDialog::action(const gcn::ActionEvent &event)
{
    setActionEventId(event.getId());
    distributeActionEvent();

    if (event.getId() == "CANCEL" || event.getSource() == mOkButton)
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
