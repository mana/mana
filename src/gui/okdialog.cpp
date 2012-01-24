/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "gui/okdialog.h"

#include "gui/gui.h"

#include "gui/widgets/button.h"
#include "gui/widgets/textbox.h"

#include "utils/gettext.h"

#include <guichan/font.hpp>

OkDialog::OkDialog(const std::string &title, const std::string &msg,
                   bool modal, Window *parent):
    Window(title, modal, parent)
{
    mTextBox = new TextBox;
    mTextBox->setEditable(false);
    mTextBox->setOpaque(false);
    mTextBox->setTextWrapped(msg, 260);

    gcn::Button *okButton = new Button(_("OK"), "ok", this);

    const int numRows = mTextBox->getNumberOfRows();
    const int fontHeight = getFont()->getHeight();
    const int height = numRows * fontHeight;
    int width = getFont()->getWidth(title);

    if (width < mTextBox->getMinWidth())
        width = mTextBox->getMinWidth();
    if (width < okButton->getWidth())
        width = okButton->getWidth();

    setContentSize(mTextBox->getMinWidth() + fontHeight, height +
                   fontHeight + okButton->getHeight());
    mTextBox->setPosition(getPadding(), getPadding());

    // 8 is the padding that GUIChan adds to button widgets
    // (top and bottom combined)
    okButton->setPosition((width - okButton->getWidth()) / 2, height + 8);

    add(mTextBox);
    add(okButton);

    center();
    setVisible(true);
    okButton->requestFocus();
}

void OkDialog::action(const gcn::ActionEvent &event)
{
    setActionEventId(event.getId());
    distributeActionEvent();

    // Can we receive anything else anyway?
    if (event.getId() == "ok")
        scheduleDelete();
}
