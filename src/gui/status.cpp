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

#include "status.h"
#include "../main.h"
#include "button.h"
#include "../engine.h"

StatusWindow::StatusWindow():
    Window("%s Lvl: % 2i Job: % 2i")
{
    #define WIN_BORDER 5
    #define CONTROLS_SEPARATOR 4
    
    hp = new gcn::Label("HP");
    sp = new gcn::Label("SP");
    hpValue = new gcn::Label();
    spValue = new gcn::Label();
    gp = new gcn::Label("GP");
    expLabel = new gcn::Label("Exp");
    jobExpLabel = new gcn::Label("Job");
    
    statsButton = new Button("Stats");
    statsButton->setEventId("Stats");
    statsButton->addActionListener(this);
    
    skillsButton = new Button("Skills");
    skillsButton->setEventId("Skills");
    skillsButton->addActionListener(this);
    
    inventoryButton = new Button("Inventory");
    inventoryButton->setEventId("Inventory");
    inventoryButton->addActionListener(this);
    
    setupButton = new Button("Setup");
    setupButton->setEventId("Setup");
    setupButton->addActionListener(this);

    equipmentButton = new Button("Equip.");
    equipmentButton->setEventId("Equipment");
    equipmentButton->addActionListener(this);
    
    hp->setPosition(WIN_BORDER, WIN_BORDER);
    sp->setPosition(WIN_BORDER, hp->getY() + hp->getHeight() + CONTROLS_SEPARATOR);
    healthBar = new ProgressBar(1.0f, WIN_BORDER + hp->getWidth() + CONTROLS_SEPARATOR, WIN_BORDER, 80, 15, 0, 255, 0);
    hpValue->setPosition(healthBar->getX() + healthBar->getWidth() + 2*CONTROLS_SEPARATOR, WIN_BORDER);
    manaBar = new ProgressBar(1.0f, WIN_BORDER + sp->getWidth() + CONTROLS_SEPARATOR,
        hp->getY() + hp->getHeight() + CONTROLS_SEPARATOR, 80, 15, 0, 0, 255);
    spValue->setPosition(manaBar->getX() + manaBar->getWidth() + 2*CONTROLS_SEPARATOR, hp->getY() + hp->getHeight() + CONTROLS_SEPARATOR);
    gp->setPosition(170, WIN_BORDER);
    expLabel->setPosition(WIN_BORDER, sp->getY() + sp->getHeight() + CONTROLS_SEPARATOR);
    jobExpLabel->setPosition(spValue->getX(), sp->getY() + sp->getHeight() + CONTROLS_SEPARATOR);
    
    xpBar = new ProgressBar(1.0f, WIN_BORDER, expLabel->getY() + expLabel->getHeight() + CONTROLS_SEPARATOR, 70, 15, 12, 194, 255);
    jobXpBar = new ProgressBar(1.0f, spValue->getX(), jobExpLabel->getY() + jobExpLabel->getHeight() + CONTROLS_SEPARATOR, 70, 15, 200, 0, 0);
    
    statsButton->setPosition(WIN_BORDER, xpBar->getY() + xpBar->getHeight() + 2*CONTROLS_SEPARATOR);
    skillsButton->setPosition(statsButton->getX() + statsButton->getWidth() + CONTROLS_SEPARATOR, statsButton->getY());
    inventoryButton->setPosition(skillsButton->getX() + skillsButton->getWidth() + CONTROLS_SEPARATOR, statsButton->getY());
    setupButton->setPosition(inventoryButton->getX() + inventoryButton->getWidth() + CONTROLS_SEPARATOR, statsButton->getY());
    equipmentButton->setPosition(setupButton->getX() + setupButton->getWidth() + CONTROLS_SEPARATOR, statsButton->getY());
    
    setContentSize(250, statsButton->getY() + statsButton->getHeight() + WIN_BORDER);
    
    add(hp);
    add(sp);
    add(healthBar);
    add(manaBar);
    add(hpValue);
    add(spValue);
    add(gp);
    add(expLabel);
    add(jobExpLabel);
    add(xpBar);
    add(jobXpBar);
    add(statsButton);
    add(skillsButton);
    add(inventoryButton);
    add(setupButton);
    add(equipmentButton);
}

StatusWindow::~StatusWindow()
{
    delete hp;
    delete sp;
    delete gp;
    delete expLabel;
    delete jobExpLabel;
    delete healthBar;
    delete manaBar;
    delete xpBar;
    delete jobXpBar;
    delete statsButton;
    delete skillsButton;
    delete inventoryButton;
    delete setupButton;
    delete equipmentButton;
}

void StatusWindow::update()
{
    char *tempstr = new char[64];

    sprintf(tempstr, "%s Lvl: % 2i Job: % 2i",
            char_info->name, char_info->lv, char_info->job_lv);
    setCaption(tempstr);

    sprintf(tempstr, "%d / %d", char_info->hp, char_info->max_hp);
    hpValue->setCaption(tempstr);
    hpValue->adjustSize();

    sprintf(tempstr, "GP %6i", char_info->gp);
    gp->setCaption(tempstr);
    gp->adjustSize();

    sprintf(tempstr, "%d / %d", char_info->sp, char_info->max_sp);
    spValue->setCaption(tempstr);
    spValue->adjustSize();
    
    sprintf(tempstr, "Exp: %d / %d",
            (int)char_info->xp, (int)char_info->xpForNextLevel);
    expLabel->setCaption(tempstr);
    expLabel->adjustSize();
    
    sprintf(tempstr, "Job Exp: %d / %d",
            (int)char_info->job_xp, (int)char_info->jobXpForNextLevel);
    jobExpLabel->setCaption(tempstr);
    jobExpLabel->adjustSize();
    
    if (char_info->hp < int(char_info->max_hp / 3))
    {
        healthBar->setColor(255, 0, 0); // Red
    }
    else
    {
        if (char_info->hp < int((char_info->max_hp / 3) * 2))
        {
            healthBar->setColor(255, 181, 9); // orange
        }
        else
        {
            healthBar->setColor(0, 255, 0); // Green
        }
    }
    
    
    healthBar->setProgress((float)char_info->hp / (float)char_info->max_hp);
    
    xpBar->setProgress((float)char_info->xp / (float)char_info->xpForNextLevel);
    jobXpBar->setProgress(
            (float)char_info->job_xp / (float)char_info->jobXpForNextLevel);

    delete[] tempstr;
}

void StatusWindow::action(const std::string& eventId) {

    if (eventId == "Stats") {
        // Show / Hide the stats dialog
	statsWindow->setVisible(!statsWindow->isVisible());
    }
    if (eventId == "Skills") {
        // Show / Hide the skills dialog
	skillDialog->setVisible(!skillDialog->isVisible());
    }
    if (eventId == "Inventory") {
        // Show / Hide the inventory dialog
	inventoryWindow->setVisible(!inventoryWindow->isVisible());
    }
    if (eventId == "Setup") {
        // Show / Hide the inventory dialog
	setupWindow->setVisible(true);
    }
    if (eventId == "Equipment") {
        // Show / Hide the inventory dialog
    equipmentWindow->setVisible(!equipmentWindow->isVisible());
    }
}
