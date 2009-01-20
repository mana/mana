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

#include <string>

#include "npc_text.h"
#include "browserbox.h"
#include "button.h"
#include "scrollarea.h"

#include "widgets/layout.h"

#include "../npc.h"

#include "../utils/gettext.h"

NpcTextDialog::NpcTextDialog():
    Window(_("NPC"))
{
    setResizable(true);

    setMinWidth(200);
    setMinHeight(150);

    setDefaultSize(0, 0, 260, 175);

    mBrowserBox = new BrowserBox(BrowserBox::AUTO_WRAP);
    mBrowserBox->setOpaque(true);

    scrollArea = new ScrollArea(mBrowserBox);
    okButton = new Button(_("OK"), "ok", this);

    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);

    place(0, 0, scrollArea, 5).setPadding(3);
    place(4, 1, okButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    loadWindowState();
    setLocationRelativeTo(getParent());
}

void NpcTextDialog::clearText()
{
    mBrowserBox->clearRows();
}

void NpcTextDialog::setText(const std::string &text)
{
    mBrowserBox->clearRows();
    mBrowserBox->addRow(text);
}

void NpcTextDialog::addText(const std::string &text)
{
    mBrowserBox->addRow(text);
}

void NpcTextDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        clearText();
        setVisible(false);
        if (current_npc)
            current_npc->nextDialog();
        current_npc = 0;
    }
}
