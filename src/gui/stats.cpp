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

#include "stats.h"

StatsDialog::StatsDialog(gcn::Container *parent):
    Window(parent, "%s Lvl: % 2i Job: % 2i")
{
    hp = new gcn::Label("HP");
    sp = new gcn::Label("SP");
    gp = new gcn::Label("GP");
    healthBar = new ProgressBar(1.0f);
    manaBar = new ProgressBar(1.0f);

    setSize(270, 40);
    hp->setPosition(6, 20);
    sp->setPosition(106, 20);
    gp->setPosition(206, 20);
    healthBar->setDimension(gcn::Rectangle(16, 6, 60, 18));
    manaBar->setDimension(gcn::Rectangle(116, 6, 60, 18));

    add(hp);
    add(sp);
    add(gp);
    add(healthBar);
    add(manaBar);
}

StatsDialog::~StatsDialog()
{
    delete hp;
    delete sp;
    delete gp;
    delete healthBar;
    delete manaBar;
}

void StatsDialog::update()
{
    char *tempstr = new char[64];

    sprintf(tempstr, "%s Lvl: % 2i Job: % 2i",
            char_info->name, char_info->lv, char_info->job_lv);
    setTitle(tempstr);

    sprintf(tempstr, "HP % 4d / % 4d", char_info->hp, char_info->max_hp);
    hp->setCaption(tempstr);
    hp->adjustSize();

    sprintf(tempstr, "GP % 6i", char_info->gp);
    gp->setCaption(tempstr);
    gp->adjustSize();

    sprintf(tempstr, "SP % 4d / % 4d", char_info->sp, char_info->max_sp);
    sp->setCaption(tempstr);
    sp->adjustSize();

    healthBar->setProgress((float)char_info->hp / (float)char_info->max_hp);

    delete tempstr;
}
