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

#include <guichan/widgets/listbox.hpp>

#include "button.h"
#include "chat.h"
#include "scrollarea.h"

#include "../resources/buddylist.h"

extern ChatWindow *chatWindow;

BuddyWindow::BuddyWindow():
    Window("Buddy")
{
    setContentSize(124, 202);

    mBuddyList = new BuddyList();

    mListbox = new gcn::ListBox();
    mListbox->setListModel(mBuddyList);

    ScrollArea *scrollArea = new ScrollArea(mListbox);
    scrollArea->setDimension(gcn::Rectangle(
                7, 5, 110, 170));
    add(scrollArea);

    Button *talk = new Button("Talk", "Talk", this);
    Button *remove = new Button("Remove", "Remove", this);
    Button *cancel = new Button("Cancel", "Cancel", this);

    talk->setPosition(2,180);
    remove->setPosition(talk->getWidth()+2,180);
    cancel->setPosition(talk->getWidth()+remove->getWidth()+2,180);

    add(talk);
    add(remove);
    add(cancel);
}

void BuddyWindow::action(const std::string& eventId, gcn::Widget* widget)
{
    if (eventId == "Talk") {
        int selected = mListbox->getSelected();
        if ( selected > -1 )
        {
            std::string who = mBuddyList->getElementAt(selected);
            chatWindow->setInputText(who +": ");
        }
    }
    else if (eventId == "Remove") {
        int selected = mListbox->getSelected();
        if ( selected > -1 )
        {
            std::string who = mBuddyList->getElementAt(selected);
            mBuddyList->removeBuddy(who);
        }
    }
    else if (eventId == "Cancel") {
        setVisible(false);
    }
}
