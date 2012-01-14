/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "actorspritemanager.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/textfield.h"

#include "utils/gettext.h"

int TextDialog::instances = 0;

TextDialog::TextDialog(const std::string &title, const std::string &msg,
                       Window *parent, bool autoCompleteEnabled):
    Window(title, true, parent)
{
    gcn::Label *textLabel = new Label(msg);
    mOkButton = new Button(_("OK"), "OK", this);
    gcn::Button *cancelButton = new Button(_("Cancel"), "CANCEL", this);

    // In TextField the escape key will either cause autoComplete or lose focus
    mTextField = new TextField(std::string(), ! autoCompleteEnabled);
    if (autoCompleteEnabled)
        mTextField->setAutoComplete(actorSpriteManager->getPlayerNameLister());

    mTextField->addActionListener(this);

    place(0, 0, textLabel, 4);
    place(0, 1, mTextField, 4);
    place(2, 2, mOkButton);
    place(3, 2, cancelButton);

    reflowLayout(textLabel->getWidth() + 20);

    if (getParent())
    {
        center();
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
    if (event.getId() == "CANCEL")
        setActionEventId("~" + getActionEventId());

    distributeActionEvent();
    scheduleDelete();
}

const std::string &TextDialog::getText() const
{
    return mTextField->getText();
}
