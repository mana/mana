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

#include <guichan/widgets/label.hpp>
#include <sstream>

#include "button.h"
#include "equipmentwindow.h"
#include "inventorywindow.h"
#include "status.h"
#include "skill.h"

extern Window *setupWindow;


MenuWindow::MenuWindow():
    Window("")
{
    setResizable(false);
    setMovable(false);
    setTitleBarHeight(0);

    // Buttons
    // ------------
    statusButton = new Button("Status");
    statusButton->setEventId("Status");
    statusButton->addActionListener(this);
    statusButton->setPosition(0, 3);
    add(statusButton);

    equipmentButton = new Button("Equipment");
    equipmentButton->setEventId("Equipment");
    equipmentButton->addActionListener(this);
    equipmentButton->setPosition(statusButton->getX() + statusButton->getWidth() + 3, statusButton->getY());
    add(equipmentButton);

    inventoryButton = new Button("Inventory");
    inventoryButton->setEventId("Inventory");
    inventoryButton->addActionListener(this);
    inventoryButton->setPosition(equipmentButton->getX() + equipmentButton->getWidth() + 3, statusButton->getY());
    add(inventoryButton);

    skillsButton = new Button("Skills");
    skillsButton->setEventId("Skills");
    skillsButton->addActionListener(this);
    skillsButton->setPosition(inventoryButton->getX() + inventoryButton->getWidth() + 3, statusButton->getY());
    add(skillsButton);

    setupButton = new Button("Setup");
    setupButton->setEventId("Setup");
    setupButton->addActionListener(this);
    setupButton->setPosition(skillsButton->getX() + skillsButton->getWidth() + 3, statusButton->getY());
    add(setupButton);

    setContentSize(setupButton->getX() + setupButton->getWidth(), setupButton->getY() + setupButton->getHeight());
}

MenuWindow::~MenuWindow()
{
    delete statusButton;
    delete equipmentButton;
    delete skillsButton;
    delete inventoryButton;
    delete setupButton;
}

void MenuWindow::draw(gcn::Graphics *graphics)
{
    Window::drawContent(graphics);
}

void MenuWindow::action(const std::string& eventId)
{
    if (eventId == "Status")
    {
        statusWindow->setVisible(!statusWindow->isVisible());
    }
    if (eventId == "Equipment")
    {
        equipmentWindow->setVisible(!equipmentWindow->isVisible());
    }
    if (eventId == "Inventory")
    {
        inventoryWindow->setVisible(!inventoryWindow->isVisible());
    }
    if (eventId == "Skills")
    {
        skillDialog->setVisible(!skillDialog->isVisible());
    }
    if (eventId == "Setup")
    {
        setupWindow->setVisible(!setupWindow->isVisible());
    }
}
