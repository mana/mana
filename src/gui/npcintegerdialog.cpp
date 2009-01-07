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
#include "textfield.h"

#include "../npc.h"

#include "../utils/gettext.h"
#include "../utils/tostring.h"

#include "widgets/layout.h"

NpcIntegerDialog::NpcIntegerDialog():
    Window(_("NPC Number Request"))
{
    mDecButton = new Button("-", "decvalue", this);
    mIncButton = new Button("+", "incvalue", this);
    mValueField = new TextField();
    okButton = new Button(_("OK"), "ok", this);
    cancelButton = new Button(_("Cancel"), "cancel", this);

    mDecButton->setSize(20, 20);
    mIncButton->setSize(20, 20);

    place(0, 0, mDecButton);
    place(1, 0, mValueField, 3);
    place(4, 0, mIncButton);
    place(2, 1, okButton);
    place(3, 1, cancelButton, 2);
    reflowLayout(175, 0);

    setLocationRelativeTo(getParent());

    mValueField->setActionEventId("valuefield");
    mValueField->addKeyListener(this);
}

void NpcIntegerDialog::prepDialog(const int min, const int def, const int max)
{
    mMin = min;
    mMax = max;
    mDefault = def;
    mValue = def;

    mValueField->setText(toString(mValue));
}

int NpcIntegerDialog::getValue()
{
    return mValue;
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
        mValue = mDefault;
    }
    else if (event.getId() == "decvalue" && mValue < mMin)
    {
        mValue--;
    }
    else if (event.getId() == "incvalue" && mValue > mMax)
    {
        mValue++;
    }

    mValueField->setText(toString(mValue));

    if (finish)
    {
        setVisible(false);
        current_npc->integerInput(mValue);
        current_npc = 0;
    }
}

void NpcIntegerDialog::keyPressed(gcn::KeyEvent &event)
{
    std::stringstream tempValue(mValueField->getText());
    int value;
    tempValue >> value;
    if (value < mMin)
    {
        value = mMin;
    }
    if (value > mMax)
    {
        value = mMax;
    }

    mValue = value;
    mValueField->setText(toString(value));
}
