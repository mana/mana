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

StatusWindow::StatusWindow():
    Window("%s Lvl: % 2i Job: % 2i")
{
    hp = new gcn::Label("HP");
    sp = new gcn::Label("SP");
    hpValue = new gcn::Label();
    spValue = new gcn::Label();
    gp = new gcn::Label("GP");
    expLabel = new gcn::Label("Exp");
    jobExpLabel = new gcn::Label("Job");
    
    hp->setPosition(5, 5);
    sp->setPosition(5, hp->getY() + hp->getHeight() + 5);
    //healthBar->setDimension(gcn::Rectangle(25, hp->getY() + 1, 60, 18));
    //manaBar->setDimension(gcn::Rectangle(25, sp->getY() + 1, 60, 18));
    hpValue->setPosition(120, 5);
    spValue->setPosition(120, hp->getY() + hp->getHeight() + 5);
    gp->setPosition(180, 20);
    expLabel->setPosition(6, 40);
    jobExpLabel->setPosition(106, 40);
    //xpBar->setDimension(gcn::Rectangle(16, 55, 60, 18));
    //jobXpBar->setDimension(gcn::Rectangle(116, 55, 60, 18));
	
    healthBar = new ProgressBar(1.0f, 20, 8, 80, 0, 255, 0);
    manaBar = new ProgressBar(1.0f, 20, 26, 80, 0, 0, 255);
    xpBar = new ProgressBar(1.0f, 6, 55, 70, 12, 194, 255);
    jobXpBar = new ProgressBar(1.0f, 106, 55, 70, 200, 0, 0);
    setSize(250, 70);
    
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
}

void StatusWindow::update()
{
    char *tempstr = new char[64];

    sprintf(tempstr, "%s Lvl: % 2i Job: % 2i",
            char_info->name, char_info->lv, char_info->job_lv);
    setTitle(tempstr);

    sprintf(tempstr, "%d / %d", char_info->hp, char_info->max_hp);
    hpValue->setCaption(tempstr);
    hpValue->adjustSize();

    sprintf(tempstr, "GP %6i", char_info->gp);
    gp->setCaption(tempstr);
    gp->adjustSize();

    sprintf(tempstr, "%d / %d", char_info->sp, char_info->max_sp);
    spValue->setCaption(tempstr);
    spValue->adjustSize();
    
    sprintf(tempstr, "Exp: %d / %d", (int)char_info->xp, (int)char_info->xpForNextLevel);
    expLabel->setCaption(tempstr);
    expLabel->adjustSize();
    
    sprintf(tempstr, "Job Exp: %d / %d", (int)char_info->job_xp, (int)char_info->jobXpForNextLevel);
    jobExpLabel->setCaption(tempstr);
    jobExpLabel->adjustSize();

    if ( char_info->hp < int(char_info->max_hp / 3) )
    {
    	healthBar->setColor(255, 0, 0); // Red
    }
    else
    {
    	if ( char_info->hp < int( (char_info->max_hp / 3)*2 ) )
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
    jobXpBar->setProgress((float)char_info->job_xp / (float)char_info->jobXpForNextLevel);

    delete tempstr;
}
