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

#include "gui/ok_dialog.h"

#include "gui/gui.h"

#include "gui/widgets/button.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/textbox.h"

#include "utils/gettext.h"

#include <guichan/font.hpp>

OkDialog::OkDialog(const std::string &title, const std::string &msg,
                   Window *parent):
    Window(title, true, parent)
{
    mTextBox = new TextBox;
    mTextBox->setEditable(false);
    mTextBox->setOpaque(false);

    mTextArea = new ScrollArea(mTextBox);
    gcn::Button *okButton = new Button(_("Ok"), "ok", this);

    mTextArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mTextArea->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mTextArea->setOpaque(false);

    mTextBox->setTextWrapped(msg, 260);

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
    mTextArea->setDimension(gcn::Rectangle(4, 5, width + 2 * getPadding(),
                                           height + getPadding()));

    // 8 is the padding that GUIChan adds to button widgets
    // (top and bottom combined)
    okButton->setPosition((width - okButton->getWidth()) / 2, height + 8);

    add(mTextArea);
    add(okButton);

    center();
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
    if (event.getId() == "ok")
        scheduleDelete();
}
