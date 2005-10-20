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

#include "menuwindow.h"

#include <string>

#include <guichan/actionlistener.hpp>

#include "button.h"

#include "../graphics.h"

extern Graphics *graphics;
extern Window *setupWindow, *inventoryWindow, *equipmentWindow,
       *skillDialog, *statusWindow;

class MenuWindowListener : public gcn::ActionListener
{
         /**
         * Called when receiving actions from widget.
         */
        void action(const std::string& eventId);
} menuWindowListener;

MenuWindow::MenuWindow():
    Window("")
{
    setResizable(false);
    setWindowName("Menu");
    setMovable(false);
    setTitleBarHeight(0);

    // Buttons
    // ------------
    gcn::Button *statusButton, *equipmentButton, *skillsButton,
        *inventoryButton, *setupButton;

    statusButton = new Button("Status");
    statusButton->setEventId("Status");
    statusButton->addActionListener(&menuWindowListener);
    statusButton->setPosition(0, 3);
    add(statusButton);

    equipmentButton = new Button("Equipment");
    equipmentButton->setEventId("Equipment");
    equipmentButton->addActionListener(&menuWindowListener);
    equipmentButton->setPosition(statusButton->getX() + statusButton->getWidth() + 3, statusButton->getY());
    add(equipmentButton);

    inventoryButton = new Button("Inventory");
    inventoryButton->setEventId("Inventory");
    inventoryButton->addActionListener(&menuWindowListener);
    inventoryButton->setPosition(equipmentButton->getX() + equipmentButton->getWidth() + 3, statusButton->getY());
    add(inventoryButton);

    skillsButton = new Button("Skills");
    skillsButton->setEventId("Skills");
    skillsButton->addActionListener(&menuWindowListener);
    skillsButton->setPosition(inventoryButton->getX() + inventoryButton->getWidth() + 3, statusButton->getY());
    add(skillsButton);

    setupButton = new Button("Setup");
    setupButton->setEventId("Setup");
    setupButton->addActionListener(&menuWindowListener);
    setupButton->setPosition(skillsButton->getX() + skillsButton->getWidth() + 3, statusButton->getY());
    add(setupButton);

    int menuWidth = setupButton->getX() + setupButton->getWidth();
    setDefaultSize((graphics->getWidth() - menuWidth - 5), 0,
                   menuWidth,
                   (setupButton->getY() + setupButton->getHeight()));
}

void MenuWindow::draw(gcn::Graphics *g)
{
    Window::drawContent(g);
}


void MenuWindowListener::action(const std::string& eventId)
{
    Window *window;
    if (eventId == "Status")
    {
        window = statusWindow;
    }
    else if (eventId == "Equipment")
    {
        window = equipmentWindow;
    }
    else if (eventId == "Inventory")
    {
        window = inventoryWindow;
    }
    else if (eventId == "Skills")
    {
        window = skillDialog;
    }
    else if (eventId == "Setup")
    {
        window = setupWindow;
    }

    if (window) {
        window->setVisible(!window->isVisible());
        if (window->isVisible()) {
            window->requestMoveToTop();
        }
    }
}
