/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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
#include "npcstringdialog.h"
#include "textfield.h"

#include "widgets/layout.h"

#include "../npc.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../utils/gettext.h"

NpcStringDialog::NpcStringDialog(Network *network):
    Window(_("NPC Text Request")), mNetwork(network)
{
    setWindowName("NPCInput");
    mValueField = new TextField("");

    setDefaultSize(175, 75, ImageRect::CENTER);

    okButton = new Button(_("OK"), "ok", this);
    cancelButton = new Button(_("Cancel"), "cancel", this);

    place(0, 0, mValueField, 3);
    place(1, 1, cancelButton);
    place(2, 1, okButton);

    loadWindowState();
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
        mValueField->setText("");

    setVisible(false);
    NPC::mTalking = false;

    std::string text = mValueField->getText();
    mValueField->setText("");

    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_STR_RESPONSE);
    outMsg.writeInt16(text.length() + 9);
    outMsg.writeInt32(current_npc);
    outMsg.writeString(text, text.length());
    outMsg.writeInt8(0);

    current_npc = 0;
}

bool NpcStringDialog::isInputFocused()
{
    return mValueField->isFocused();
}

void NpcStringDialog::requestFocus()
{
    mValueField->requestFocus();
}
