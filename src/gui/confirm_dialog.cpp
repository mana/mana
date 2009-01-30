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

#include "confirm_dialog.h"

#include "../utils/gettext.h"

ConfirmDialog::ConfirmDialog(const std::string &title, const std::string &msg,
        Window *parent):
    Window(title, true, parent)
{
    mTextBox = new TextBox();
    mTextBox->setEditable(false);
    mTextBox->setOpaque(false);

    mTextArea = new ScrollArea(mTextBox);
    gcn::Button *yesButton = new Button(_("Yes"), "yes", this);
    gcn::Button *noButton = new Button(_("No"), "no", this);

    mTextArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mTextArea->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mTextArea->setOpaque(false);

    mTextBox->setMinWidth(260);
    mTextBox->setTextWrapped(msg);

    int numRows = mTextBox->getNumberOfRows();
    int width = getFont()->getWidth(title);
    int inWidth = yesButton->getWidth() + noButton->getWidth() + 5;

    if (numRows > 1)
    {
        // 15 == height of each line of text (based on font heights)
        // 14 == row top + bottom graphic pixel heights
        setContentSize(mTextBox->getMinWidth() + 15, 15 + (numRows * 15) + noButton->getHeight());
        mTextArea->setDimension(gcn::Rectangle(4, 5, mTextBox->getMinWidth() + 5, 
                                               3 + (numRows * 14)));
    }
    else
    {
        if (width < getFont()->getWidth(msg))
            width = getFont()->getWidth(msg);
        if (width < inWidth)
            width = inWidth;
        setContentSize(width + 15, 30 + noButton->getHeight());
        mTextArea->setDimension(gcn::Rectangle(4, 5, width + 5, 17));
    }

    yesButton->setPosition(
            (mTextBox->getMinWidth() - inWidth) / 2,
            (numRows * 14) + noButton->getHeight() - 8);
    noButton->setPosition(
            yesButton->getX() + yesButton->getWidth() + 5,
            (numRows * 14) + noButton->getHeight() - 8);

    add(mTextArea);
    add(yesButton);
    add(noButton);

    if (getParent()) {
        setLocationRelativeTo(getParent());
        getParent()->moveToTop(this);
    }
    setVisible(true);
    yesButton->requestFocus();
}

unsigned int ConfirmDialog::getNumRows()
{
    return mTextBox->getNumberOfRows();
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
