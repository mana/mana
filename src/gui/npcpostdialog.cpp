/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/npcpostdialog.h"

#include "event.h"
#include "playerinfo.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/textbox.h"
#include "gui/widgets/textfield.h"
#include "gui/widgets/scrollarea.h"

#include "utils/gettext.h"

NpcPostDialog::DialogList NpcPostDialog::instances;

NpcPostDialog::NpcPostDialog(int npcId):
    Window(_("NPC")),
    mNpcId(npcId)
{
    setContentSize(400, 180);

    // create text field for receiver
    gcn::Label *senderText = new Label(_("To:"));
    senderText->setPosition(5, 5);
    mSender = new TextField;
    mSender->setPosition(senderText->getWidth() + 5, 5);
    mSender->setWidth(65);

    // create button for sending
    Button *sendButton = new Button(_("Send"), "send", this);
    sendButton->setPosition(400 - sendButton->getWidth(),
                            170 - sendButton->getHeight());
    Button *cancelButton = new Button(_("Cancel"), "cancel", this);
    cancelButton->setPosition(sendButton->getX() - (cancelButton->getWidth() + 2),
                              sendButton->getY());

    // create textfield for letter
    mText = new TextBox;
    mText->setHeight(400 - (mSender->getHeight() + sendButton->getHeight()));
    mText->setEditable(true);

    // create scroll box for letter text
    ScrollArea *scrollArea = new ScrollArea(mText);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setDimension(gcn::Rectangle(
                5, mSender->getHeight() + 5,
                380, 140 - (mSender->getHeight() + sendButton->getHeight())));

    add(senderText);
    add(mSender);
    add(scrollArea);
    add(sendButton);
    add(cancelButton);

    setLocationRelativeTo(getParent());

    instances.push_back(this);
    setVisible(true);

    PlayerInfo::setNPCPostCount(PlayerInfo::getNPCPostCount() + 1);
}

NpcPostDialog::~NpcPostDialog()
{
    instances.remove(this);
    PlayerInfo::setNPCPostCount(PlayerInfo::getNPCPostCount() - 1);
}

void NpcPostDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "send")
    {
        if (mSender->getText().empty() || mText->getText().empty())
        {
            SERVER_NOTICE(_("Failed to send as sender or letter invalid."))
        }
        else
        {
            Mana::Event event("doSendLetter");
            event.setInt("npcId", mNpcId);
            event.setString("recipient", mSender->getText());
            event.setString("text", mText->getText());
            event.trigger(CHANNEL_NPC);
        }
        setVisible(false);
    }
    else if (event.getId() == "cancel")
    {
        setVisible(false);
    }
}

void NpcPostDialog::setVisible(bool visible)
{
    Window::setVisible(visible);

    if (!visible)
    {
        scheduleDelete();
    }
}

void NpcPostDialog::closeAll()
{
    DialogList::iterator it = instances.begin();
    DialogList::iterator it_end = instances.end();

    for (; it != it_end; it++)
    {
        (*it)->close();
    }
}
