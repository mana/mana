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

#include "button.h"
#include "npc_text.h"
#include "npcstringdialog.h"
#include "textfield.h"

#include "widgets/layout.h"

#include "../npc.h"

#ifdef EATHENA_SUPPORT
#include "../net/ea/npchandler.h"
#endif

#include "../utils/gettext.h"
#include "../utils/strprintf.h"

NpcStringDialog::NpcStringDialog()
    : Window(_("NPC Text Request"))
{
    setWindowName("NPCString");
    mValueField = new TextField("");

    setDefaultSize(175, 75, ImageRect::CENTER);

    gcn::Button *okButton = new Button(_("OK"), "ok", this);
    gcn::Button *cancelButton = new Button(_("Cancel"), "cancel", this);

    place(0, 0, mValueField, 3);
    place(1, 1, cancelButton);
    place(2, 1, okButton);
    reflowLayout(175, 0);

    center();
    setDefaultSize();
    loadWindowState();
}

std::string NpcStringDialog::getValue()
{
    return mValueField->getText();
}

void NpcStringDialog::setValue(const std::string &value)
{
    mValueField->setText(value);
    mDefault = value;
}

void NpcStringDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "cancel")
    {
        mValueField->setText(mDefault);
        npcTextDialog->addText(_("\n> Cancel\n"));
    }
    else
    {
        npcTextDialog->addText(strprintf("\n> \"%s\"\n",
                                          mValueField->getText().c_str()));
    }

    setVisible(false);
    NPC::isTalking = false;

    std::string text = mValueField->getText();
    mValueField->setText("");

    // Net::getNpcHandler()->stringInput(current_npc, text);
#ifdef EATHENA_SUPPORT
        npcHandler->stringInput(current_npc, text);
#endif
}

bool NpcStringDialog::isInputFocused()
{
    return mValueField->isFocused();
}

void NpcStringDialog::requestFocus()
{
    mValueField->requestFocus();
}

void NpcStringDialog::setVisible(bool visible)
{
    if (visible) {
        npcTextDialog->setVisible(true);
        requestFocus();
    }

    Window::setVisible(visible);
}
