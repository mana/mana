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

#include "gui/buddywindow.h"

#include "gui/chat.h"

#include "gui/widgets/avatar.h"
#include "gui/widgets/icon.h"

#include "resources/resourcemanager.h"
#include "utils/gettext.h"

extern ChatWindow *chatWindow;

BuddyWindow::BuddyWindow():
    Window(_("Buddy"))
{
    setWindowName("BuddyWindow");
    setCaption(_("Buddy List"));
    setResizable(true);
    setCloseButton(true);
    setSaveVisible(true);
    setMinWidth(110);
    setMinHeight(200);
    setDefaultSize(124, 41, 288, 330);

    Image *addImg = ResourceManager::getInstance()->getImage("buddyadd.png");
    Image *delImg = ResourceManager::getInstance()->getImage("buddydel.png");

    if (addImg && delImg)
    {
        Icon *addBuddy = new Icon(addImg);
        Icon *delBuddy = new Icon(delImg);

        add(addBuddy);
        add(delBuddy);
    }

    loadWindowState();
}

void BuddyWindow::action(const gcn::ActionEvent &event)
{

}

void BuddyWindow::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);
}
