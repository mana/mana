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

#include "buddywindow.h"
#include "chat.h"
#include "icon.h"

#include "widgets/avatar.h"

#include "../resources/resourcemanager.h"
#include "../utils/gettext.h"

extern ChatWindow *chatWindow;

BuddyWindow::BuddyWindow():
    Window(_("Buddy"))
{
    setVisible(false);
    setContentSize(124, 202);
    setCaption(_("Buddy List"));
    setResizable(true);
    setCloseButton(true);

    Image *addImg = ResourceManager::getInstance()->getImage("buddyadd.png");
    Image *delImg = ResourceManager::getInstance()->getImage("buddydel.png");

    if (addImg && delImg)
    {
        Icon *addBuddy = new Icon(addImg);
        Icon *delBuddy = new Icon(delImg);

        add(addBuddy);
        add(delBuddy);
    }

    loadWindowState("Buddy");
}

void BuddyWindow::action(const gcn::ActionEvent &event)
{

}
