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

#include "npcstringdialog.h"

#include <limits>
#include <sstream>

#include "button.h"
#include "textfield.h"

#include "../npc.h"

#include "../utils/gettext.h"
#include "../utils/tostring.h"

#include "widgets/layout.h"

NpcStringDialog::NpcStringDialog():
    Window(_("NPC Text Request"))
{
    mValueField = new TextField("");
    okButton = new Button(_("OK"), "ok", this);
    cancelButton = new Button(_("Cancel"), "cancel", this);

    place(0, 0, mValueField, 3);
    place(1, 1, cancelButton);
    place(2, 1, okButton);
    reflowLayout(175, 0);

    setLocationRelativeTo(getParent());
}

std::string NpcStringDialog::getValue()
{
    return mValueField->getText();
}

void NpcStringDialog::setValue(const std::string &value)
{
    mValueField->setText(value);
}

void NpcStringDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "cancel")
    {
        mValueField->setText("");
    }

    setVisible(false);
    current_npc->stringInput(mValueField->getText());
    current_npc = 0;
    mValueField->setText("");
}

bool NpcStringDialog::isInputFocused()
{
    return mValueField->isFocused();
}
