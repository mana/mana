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

#include "gui/npc_text.h"

#include "gui/widgets/button.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/textbox.h"

#include "npc.h"

#include "net/net.h"
#include "net/npchandler.h"

#include "utils/gettext.h"

NpcTextDialog::NpcTextDialog()
    : Window(_("NPC"))
    , mState(NPC_TEXT_STATE_WAITING)
{
    setWindowName("NPCText");
    setResizable(true);

    setMinWidth(200);
    setMinHeight(150);

    setDefaultSize(260, 200, ImageRect::CENTER);

    mTextBox = new TextBox;
    mTextBox->setEditable(false);
    mTextBox->setOpaque(false);

    mScrollArea = new ScrollArea(mTextBox);
    mButton = new Button(_("Waiting for server"), "ok", this);

    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mScrollArea->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);

    place(0, 0, mScrollArea, 5).setPadding(3);
    place(4, 1, mButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    center();
    loadWindowState();
}

void NpcTextDialog::clearText()
{
    NPC::isTalking = false;
    setText("");
}

void NpcTextDialog::setText(const std::string &text)
{
    mText = text;
    mTextBox->setTextWrapped(mText, mScrollArea->getWidth() - 15);
}

void NpcTextDialog::addText(const std::string &text)
{
    setText(mText + text + "\n");
    mScrollArea->setVerticalScrollAmount(mScrollArea->getVerticalMaxScroll());
}

void NpcTextDialog::showNextButton()
{
    mButton->setCaption(_("Next"));
    mState = NPC_TEXT_STATE_NEXT;
    mButton->setEnabled(true);
}

void NpcTextDialog::showCloseButton()
{
    mButton->setCaption(_("Close"));
    mState = NPC_TEXT_STATE_CLOSE;
    mButton->setEnabled(true);
}

void NpcTextDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        if (mState == NPC_TEXT_STATE_NEXT && current_npc) {
            nextDialog();
            addText("\n> Next\n");
        } else if (mState == NPC_TEXT_STATE_CLOSE ||
                (mState == NPC_TEXT_STATE_NEXT && !current_npc)) {
            setText("");
            if (current_npc) nextDialog();
            setVisible(false);
            current_npc = 0;
            NPC::isTalking = false;
        } else return;
    }
    else return;

    mButton->setEnabled(false);
    mButton->setCaption(_("Waiting for server"));
    mState = NPC_TEXT_STATE_WAITING;
}

void NpcTextDialog::nextDialog(int npcID)
{
    Net::getNpcHandler()->nextDialog(npcID);
}

void NpcTextDialog::closeDialog(int npcID)
{
    Net::getNpcHandler()->closeDialog(npcID);
}

void NpcTextDialog::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);

    setText(mText);
}
