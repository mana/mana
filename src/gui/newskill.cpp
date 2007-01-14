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

 /* This file implements the new skill dialog for use under the latest
  * version of the skill system as of 2005/02/20
  */

#include "newskill.h"

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "progressbar.h"

#include "../graphics.h"

const char *skill_name[] = {
    // 0-99
    // weapon skills 0-9
    "Short Blades", "Long Blades", "Hammers", "Archery", "Whip",
    "Staves", "Throwing", "Piercing", "Hand to Hand", NULL,
    // magic skills 10-19
    "Epyri (Fire)", "Merene (Water)", "Geon (Earth)", "Izurial (Air)",
    "Lumine (Light)", "Tenebrae (Dark)", "Chronos (Time)", "Teless (Space)",
    "Gen (Mana)", NULL,
    // craft skills 20-29
    "Metalworking", "Woodworking", "Jeweler", "Cook", "Tailor",
    "Alchemist", "Artisan", "Synthesis", NULL, NULL,
    // general skills 30-39
    "Running", "Searching", "Sneak", "Trading", "Intimidate",
    "Athletics", NULL, NULL, NULL,NULL,
    // combat skills 40-49
    "Dodge", "Accuracy", "Critical", "Block", "Parry", "Diehard", "Magic Aura",
    "Counter", NULL, NULL,
    // resistance skills 50-59
    "Poison", "Silence", "Petrify", "Paralyze", "Blind", "Slow", "Zombie",
    "Critical", NULL, NULL,
    // element reistance 60-69
    "Heat (Fire)", "Chill (Water)", "Stone (Earth)", "Wind (Air)",
    "Shine (Light)", "Shadow (Dark)", "Decay (Time)", "Chaos (Space)", NULL,
    NULL,
    // hunting skills 70-79
    "Insects", "Birds", "Lizards", "Amorphs", "Undead", "Machines", "Arcana",
    "Humanoids", "Plantoids", NULL,
    // stats 80-89
    "Strength", "Fortitude", "Vitality", "Menality", "Awareness", "Mana",
    "Dexterity", NULL, NULL, NULL,
    // unused (reserved) 90-99
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};


NewSkillDialog::NewSkillDialog():
    Window("Skills")
{
    startPoint = 0;
    for (int i = 0; i < N_SKILL_CAT_SIZE; i++)
    {
        mSkillLabel[i] = new gcn::Label("Empty               ");
        mSkillLevel[i] = new gcn::Label("00000");
        mSkillbar[i] = new ProgressBar(0.0f,100,15,0,0,255);
        mSkillLevel[i]->setAlignment(Graphics::RIGHT);
        add(mSkillLabel[i],40,5+i*25);
        add(mSkillLevel[i],150,5+i*25);
        add(mSkillbar[i],180,5+i*25);
    }
    // initialize the skills
    for (int i = 0; i < N_SKILL; i++)
    {
        mPlayerSkill[i].level = 0;
        mPlayerSkill[i].exp = 0;
    }
    resetNSD();

    // create controls
    Button *catButton[N_SKILL_CAT];
    catButton[0] = new Button("Weapons", "g1", this);
    catButton[1] = new Button("Magic", "g2", this);
    catButton[2] = new Button("Craft", "g3", this);
    catButton[3] = new Button("General", "g4", this);
    catButton[4] = new Button("Combat", "g5", this);
    catButton[5] = new Button("E. Resist", "g6", this);
    catButton[6] = new Button("S. Resist", "g7", this);
    catButton[7] = new Button("Hunting", "g8", this);
    catButton[8] = new Button("Stat", "g9", this);

    setContentSize(350, 250);

    for (int i = 0; i < 9; ++i) {
        catButton[i]->setDimension(gcn::Rectangle(0, 0, 60, 20));
        catButton[i]->setPosition(290, 20 * i);
        add(catButton[i]);
    }

    Button *closeButton = new Button("Close", "close", this);
    closeButton->setDimension(gcn::Rectangle(0,0,60,20));
    closeButton->setPosition(290, 230);
    add(closeButton);

    // finsihing touches
    setLocationRelativeTo(getParent());
}

void NewSkillDialog::action(const gcn::ActionEvent &event)
{
     int osp = startPoint;
    if (event.getId() == "close")
    {
        setVisible(false);
    }
    else if (event.getId() == "g1") // weapons group 0-9
    {
        startPoint =0;
    }
    else if (event.getId() == "g2") // magic group 10-19
    {
        startPoint =10;
    }
    else if (event.getId() == "g3") // craft group 20-29
    {
        startPoint =20;
    }
    else if (event.getId() == "g4") // general group 30-39
    {
        startPoint =30;
    }
    else if (event.getId() == "g5") // combat group 40-49
    {
        startPoint =40;
    }
    else if (event.getId() == "g6") // e. resist group 50-59
    {
        startPoint =50;
    }
    else if (event.getId() == "g7") // s resist group 60-69
    {
        startPoint =60;
    }
    else if (event.getId() == "g8") // hunting group 70-79
    {
        startPoint =70;
    }
    else if (event.getId() == "g9") // stats group 80-89
    {
        startPoint =80;
    }
    if (osp != startPoint)
    {
        resetNSD();
    }
}

void NewSkillDialog::resetNSD()
{
     for (int a = 0; a < N_SKILL_CAT_SIZE; a++)
     {
         if (skill_name[a + startPoint])
         {
             mSkillLabel[a]->setCaption(skill_name[a + startPoint]);
             mSkillLabel[a]->setVisible(true);
             char tmp[5];
             sprintf(tmp, "%d", mPlayerSkill[a+startPoint].level);
             mSkillLevel[a]->setCaption(tmp);
             mSkillLevel[a]->setVisible(true);
             mSkillbar[a]->setProgress(0.0f);
             mSkillbar[a]->setVisible(true);
         }
         else
         {
             mSkillLevel[a]->setVisible(false);
             mSkillLabel[a]->setVisible(false);
             mSkillbar[a]->setVisible(false);
         }
     }
}
