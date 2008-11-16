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

#include "npc_text.h"

#include <string>

#include "scrollarea.h"
#include "button.h"
#include "textbox.h"

#include "../npc.h"

#include "../utils/gettext.h"

NpcTextDialog::NpcTextDialog():
    Window(_("NPC"))
{
    mTextBox = new TextBox;
    mTextBox->setEditable(false);
    gcn::ScrollArea *scrollArea = new ScrollArea(mTextBox);
    Button *okButton = new Button(_("Ok"), "ok", this);

    setContentSize(260, 175);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);
    scrollArea->setDimension(gcn::Rectangle(
                5, 5, 250, 160 - okButton->getHeight()));
    okButton->setPosition(
            260 - 5 - okButton->getWidth(),
            175 - 5 - okButton->getHeight());

    add(scrollArea);
    add(okButton);

    setLocationRelativeTo(getParent());
}

void
NpcTextDialog::setText(const std::string &text)
{
    mTextBox->setTextWrapped(text);
}

void
NpcTextDialog::addText(const std::string &text)
{
    mTextBox->setTextWrapped(mTextBox->getText() + text + "\n");
}

void
NpcTextDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        setText("");
        setVisible(false);
        if (current_npc)
            current_npc->nextDialog();
        current_npc = 0;
    }
}
