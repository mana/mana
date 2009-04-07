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

#include "gui/npcintegerdialog.h"

#include "gui/npc_text.h"

#include "gui/widgets/button.h"
#include "gui/widgets/inttextfield.h"
#include "gui/widgets/layout.h"

#include "npc.h"

#include "net/net.h"
#include "net/npchandler.h"

#include "utils/gettext.h"
#include "utils/strprintf.h"

NpcIntegerDialog::NpcIntegerDialog()
    : Window(_("NPC Number Request"))
{
    setWindowName("NPCInteger");
    mValueField = new IntTextField;

    setDefaultSize(175, 75, ImageRect::CENTER);

    mDecButton = new Button("-", "decvalue", this);
    mIncButton = new Button("+", "incvalue", this);
    gcn::Button *okButton = new Button(_("OK"), "ok", this);
    gcn::Button *cancelButton = new Button(_("Cancel"), "cancel", this);
    gcn::Button *resetButton = new Button(_("Reset"), "reset", this);

    mDecButton->adjustSize();
    mDecButton->setWidth(mIncButton->getWidth());

    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mDecButton);
    place(1, 0, mValueField, 3);
    place(4, 0, mIncButton);
    place.getCell().matchColWidth(1, 0);
    place = getPlacer(0, 1);
    place(0, 0, resetButton);
    place(2, 0, cancelButton);
    place(3, 0, okButton);
    reflowLayout(175, 0);

    center();
    setDefaultSize();
    loadWindowState();
}

void NpcIntegerDialog::setRange(int min, int max)
{
    mValueField->setRange(min, max);
}

int NpcIntegerDialog::getValue()
{
    return mValueField->getValue();
}

void NpcIntegerDialog::reset()
{
    mValueField->reset();
}

void NpcIntegerDialog::action(const gcn::ActionEvent &event)
{
    bool finish = false;

    if (event.getId() == "ok")
    {
        finish = true;
        npcTextDialog->addText(strprintf("\n> %d\n", mValueField->getValue()));
    }
    else if (event.getId() == "cancel")
    {
        finish = true;
        mValueField->reset();
        npcTextDialog->addText(_("\n> Cancel\n"));
    }
    else if (event.getId() == "decvalue")
    {
        mValueField->setValue(mValueField->getValue() - 1);
    }
    else if (event.getId() == "incvalue")
    {
        mValueField->setValue(mValueField->getValue() + 1);
    }
    else if (event.getId() == "reset")
    {
        mValueField->reset();
    }

    if (finish)
    {
        setVisible(false);
        NPC::isTalking = false;

        Net::getNpcHandler()->integerInput(current_npc, mValueField->getValue());

        mValueField->reset();
    }
}

void NpcIntegerDialog::setDefaultValue(int value)
{
    mValueField->setDefaultValue(value);
}

bool NpcIntegerDialog::isInputFocused()
{
    return mValueField->isFocused();
}

void NpcIntegerDialog::requestFocus()
{
    mValueField->requestFocus();
}

void NpcIntegerDialog::setVisible(bool visible)
{
    if (visible) {
        npcTextDialog->setVisible(true);
        requestFocus();
    }

    Window::setVisible(visible);
}
