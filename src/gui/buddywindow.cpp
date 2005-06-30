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
 *
 *  $Id$
 */

#include "buddywindow.h"
#include "scrollarea.h"
#include "button.h"
#include "chat.h"

extern ChatWindow *chatWindow;

BuddyWindow::BuddyWindow():
    Window("Buddy")
{
    setContentSize(120, 200);

    listbox = new gcn::ListBox();
    listbox->setListModel(dynamic_cast<ListModel*>(this));

    scrollArea = new ScrollArea(listbox);
    scrollArea->setDimension(gcn::Rectangle(
                2, 0, 114, 176));
    add(scrollArea);

    talk = new Button("Talk");
    talk->setPosition(1,180);
    talk->addActionListener(this);
    talk->setEventId("Talk");
    remove = new Button("Remove");
    remove->addActionListener(this);
    remove->setEventId("Remove");
    remove->setPosition(talk->getWidth()+2,180);
    cancel = new Button("Cancel");
    cancel->addActionListener(this);
    cancel->setEventId("Cancel");
    cancel->setPosition(talk->getWidth()+remove->getWidth()+2,180);

    add(talk);
    add(remove);
    add(cancel);
}

BuddyWindow::~BuddyWindow()
{
    delete listbox;
    delete scrollArea;
    delete talk;
    delete remove;
    delete cancel;
}

void BuddyWindow::action(const std::string& eventId)
{
    if (eventId == "Talk") {
        int selected = listbox->getSelected();
        if ( selected > -1 )
        {
            std::string who = getElementAt(selected);
            chatWindow->setInputText(who +": ");
        }
    }
    else if (eventId == "Remove") {
        int selected = listbox->getSelected();
        if ( selected > -1 )
        {
            std::string who = getElementAt(selected);
            removeBuddy(who);
        }
    }
    else if (eventId == "Cancel") {
        setVisible(false);
    }
}
