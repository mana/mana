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

#include "npcintegerdialog.h"

#include <limits>
#include <sstream>

#include "button.h"
#include "inttextfield.h"

#include "../npc.h"

#include "../utils/gettext.h"
#include "../utils/tostring.h"

#include "widgets/layout.h"

NpcIntegerDialog::NpcIntegerDialog():
    Window(_("NPC Number Request"))
{
    mDecButton = new Button("-", "decvalue", this);
    mIncButton = new Button("+", "incvalue", this);
    mValueField = new IntTextField();
    okButton = new Button(_("OK"), "ok", this);
    cancelButton = new Button(_("Cancel"), "cancel", this);
    resetButton = new Button(_("Reset"), "reset", this);

    mDecButton->setSize(20, 20);
    mIncButton->setSize(20, 20);

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

    setLocationRelativeTo(getParent());

    mValueField->setActionEventId("valuefield");
    mValueField->addKeyListener(this);
}

void NpcIntegerDialog::setRange(const int min, const int max)
{
    mValueField->setRange(min, max);
}

int NpcIntegerDialog::getValue()
{
    return mValueField->getValue();
}

void NpcIntegerDialog::action(const gcn::ActionEvent &event)
{
    int finish = 0;

    if (event.getId() == "ok")
    {
        finish = 1;
    }
    else if (event.getId() == "cancel")
    {
        finish = 1;
        mValueField->reset();
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
        current_npc->integerInput(mValueField->getValue());
        current_npc = 0;
    }
}
