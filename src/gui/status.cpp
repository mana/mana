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
    healthBar = new ProgressBar(1.0f);
    manaBar = new ProgressBar(1.0f);
    //xpBar = new ProgressBar(1.0f);
    //jobXpBar = new ProgressBar(1.0f);

    setSize(250, 60);
    hp->setPosition(5, 5);
    sp->setPosition(5, hp->getY() + hp->getHeight() + 5);
    healthBar->setDimension(gcn::Rectangle(25, hp->getY() + 1, 60, 18));
    manaBar->setDimension(gcn::Rectangle(25, sp->getY() + 1, 60, 18));
    hpValue->setPosition(
            healthBar->getX() + healthBar->getWidth() + 5, hp->getY());
    spValue->setPosition(
            manaBar->getX() + manaBar->getWidth() + 5, sp->getY());
    gp->setPosition(180, 20);
    expLabel->setPosition(6, 40);
    jobExpLabel->setPosition(106, 40);
    //xpBar->setDimension(gcn::Rectangle(16, 6, 60, 18));
    //jobXpBar->setDimension(gcn::Rectangle(116, 15, 60, 18));

    add(hp);
    add(sp);
    add(healthBar);
    add(manaBar);
    add(hpValue);
    add(spValue);
    add(gp);
    add(expLabel);
    add(jobExpLabel);
    //add(xpBar);
    //add(jobXpBar);
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
    //delete xpBar;
    //delete jobXpBar;
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
    
    sprintf(tempstr, "Exp: %d", (int)char_info->xp);
    expLabel->setCaption(tempstr);
    expLabel->adjustSize();
    
    sprintf(tempstr, "Job Points: %d", (int)char_info->job_xp);
    jobExpLabel->setCaption(tempstr);
    jobExpLabel->adjustSize();

    healthBar->setProgress((float)char_info->hp / (float)char_info->max_hp);

    delete tempstr;
}
