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

#include <guichan/font.hpp>

#include "button.h"
#include "gui.h"
#include "ok_dialog.h"
#include "scrollarea.h"
#include "textbox.h"

#include "../utils/gettext.h"

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

    int numRows = mTextBox->getNumberOfRows();
    const int fontHeight = getFont()->getHeight();

    if (numRows > 1)
    {
        // 14 == row top + bottom graphic pixel heights
        setContentSize(mTextBox->getMinWidth() + fontHeight, ((numRows + 1) * 
                       fontHeight) + okButton->getHeight());
        mTextArea->setDimension(gcn::Rectangle(4, 5, mTextBox->getMinWidth() + 5,
                                               3 + (numRows * fontHeight)));
    }
    else
    {
        int width = getFont()->getWidth(title);
        if (width < getFont()->getWidth(msg))
            width = getFont()->getWidth(msg);
        if (width < okButton->getWidth())
            width = okButton->getWidth();
        setContentSize(width + fontHeight, 30 + okButton->getHeight());
        mTextArea->setDimension(gcn::Rectangle(4, 5, width + 5, 17));
    }

    okButton->setPosition((mTextBox->getMinWidth() - okButton->getWidth()) / 2,
                          ((numRows - 1) * fontHeight) + okButton->getHeight() + 2);

    add(mTextArea);
    add(okButton);

    setLocationRelativeTo(getParent());
    setVisible(true);
    okButton->requestFocus();
}

unsigned int OkDialog::getNumRows()
{
    return mTextBox->getNumberOfRows();
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
