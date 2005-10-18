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
    "Staves", "Throwing", "Piercing", "Hand to Hand", "",
    // magic skills 10-19
    "Epyri (Fire)", "Merene (Water)", "Geon (Earth)", "Izurial (Air)",
    "Lumine (Light)", "Tenebrae (Dark)", "Chronos (Time)", "Teless (Space)",
    "Gen (Mana)", "",
    // craft skills 20-29
    "Metalworking", "Woodworking", "Jeweler", "Cook", "Tailor",
    "Alchemist", "Artisan", "Synthesis", "", "",
    // general skills 30-39
    "Running", "Searching", "Sneak", "Trading", "Intimidate",
    "Athletics", "", "", "","",
    // combat skills 40-49
    "Dodge", "Accuracy", "Critical", "Block", "Parry", "Diehard", "Magic Aura",
    "Counter", "", "",
    // resistance skills 50-59
    "Poison", "Silence", "Petrify", "Paralyze", "Blind", "Slow", "Zombie",
    "Critical", "", "",
    // element reistance 60-69
    "Heat (Fire)", "Chill (Water)", "Stone (Earth)", "Wind (Air)", 
    "Shine (Light)", "Shadow (Dark)", "Decay (Time)", "Chaos (Space)", "", "",
    // hunting skills 70-79
    "Insects", "Birds", "Lizards", "Amorphs", "Undead", "Machines", "Arcana",
    "Humanoids", "Plantoids", "",
    // stats 80-89
    "Strength", "Fortitude", "Vitality", "Menality", "Awareness", "Mana",
    "Dexterity", "", "", "",
    // unused (reserved) 90-99
    "", "", "", "", "", "", "", "", "", ""
};


NewSkillDialog::NewSkillDialog():
    Window("Skills")
{
    startPoint = 0;
    for ( int i = 0; i < N_SKILL_CAT_SIZE; i++)
    {
        skillLabel[i] = new gcn::Label("Empty               ");
        skillLevel[i] = new gcn::Label("00000");
        skillbar[i] = new ProgressBar(0.0f,0,0,100,15,0,0,255);
        skillLevel[i]->setAlignment(Graphics::RIGHT);
        add(skillLabel[i],40,5+i*25);
        add(skillLevel[i],150,5+i*25);
        add(skillbar[i],180,5+i*25);
    }
    // initialize the skills
    for(int i=0;i<N_SKILL;i++)
    {
        playerSkill[i].level = 0;
        playerSkill[i].exp = 0;
    }
    resetNSD();

    // create controls
    catButton[0] = new Button("Weapons");
    catButton[1] = new Button("Magic");
    catButton[2] = new Button("Craft");
    catButton[3] = new Button("General");
    catButton[4] = new Button("Combat");
    catButton[5] = new Button("E. Resist");
    catButton[6] = new Button("S. Resist");
    catButton[7] = new Button("Hunting");
    catButton[8] = new Button("Stat");
    closeButton = new Button("Close");

    // captions

    // events
    catButton[0]->setEventId("g1");
    catButton[1]->setEventId("g2");
    catButton[2]->setEventId("g3");
    catButton[3]->setEventId("g4");
    catButton[4]->setEventId("g5");
    catButton[5]->setEventId("g6");
    catButton[6]->setEventId("g7");
    catButton[7]->setEventId("g8");
    catButton[8]->setEventId("g9");
    closeButton->setEventId("close");

    // positioning
    setContentSize(350, 250);
    catButton[0]->setDimension(gcn::Rectangle(0,0,60,20));
    catButton[0]->setPosition(290, 0);
    catButton[1]->setDimension(gcn::Rectangle(0,0,60,20));
    catButton[1]->setPosition(290, 20);
    catButton[2]->setDimension(gcn::Rectangle(0,0,60,20));
    catButton[2]->setPosition(290, 40);
    catButton[3]->setDimension(gcn::Rectangle(0,0,60,20));
    catButton[3]->setPosition(290, 60);
    catButton[4]->setDimension(gcn::Rectangle(0,0,60,20));
    catButton[4]->setPosition(290, 80);
    catButton[5]->setDimension(gcn::Rectangle(0,0,60,20));
    catButton[5]->setPosition(290, 100);
    catButton[6]->setDimension(gcn::Rectangle(0,0,60,20));
    catButton[6]->setPosition(290, 120);
    catButton[7]->setDimension(gcn::Rectangle(0,0,60,20));
    catButton[7]->setPosition(290, 140);
    catButton[8]->setDimension(gcn::Rectangle(0,0,60,20));
    catButton[8]->setPosition(290, 160);
    closeButton->setDimension(gcn::Rectangle(0,0,60,20));
    closeButton->setPosition(290, 230);

    // add controls
    add(catButton[0]);
    add(catButton[1]);
    add(catButton[2]);
    add(catButton[3]);
    add(catButton[4]);
    add(catButton[5]);
    add(catButton[6]);
    add(catButton[7]);
    add(catButton[8]);
    add(closeButton);

    // add event detection
    catButton[0]->addActionListener(this);
    catButton[1]->addActionListener(this);
    catButton[2]->addActionListener(this);
    catButton[3]->addActionListener(this);
    catButton[4]->addActionListener(this);
    catButton[5]->addActionListener(this);
    catButton[6]->addActionListener(this);
    catButton[7]->addActionListener(this);
    catButton[8]->addActionListener(this);
    closeButton->addActionListener(this);

    // finsihing touches
    setLocationRelativeTo(getParent());
}

void NewSkillDialog::action(const std::string& eventId)
{
     int osp = startPoint;
    if (eventId == "close")
    {
        setVisible(false);
    }
    else if (eventId == "g1") // weapons group 0-9
    {
        startPoint =0;
    }
    else if (eventId == "g2") // magic group 10-19
    {
        startPoint =10;
    }
    else if (eventId == "g3") // craft group 20-29
    {
        startPoint =20;
    }
    else if (eventId == "g4") // general group 30-39
    {
        startPoint =30;
    }
    else if (eventId == "g5") // combat group 40-49
    {
        startPoint =40;
    }
    else if (eventId == "g6") // e. resist group 50-59
    {
        startPoint =50;
    }
    else if (eventId == "g7") // s resist group 60-69
    {
        startPoint =60;
    }
    else if (eventId == "g8") // hunting group 70-79
    {
        startPoint =70;
    }
    else if (eventId == "g9") // stats group 80-89
    {
        startPoint =80;
    }
    if(osp != startPoint)
    {
        resetNSD();
    }
}

void NewSkillDialog::resetNSD()
{
     for(int a=0;a<N_SKILL_CAT_SIZE;a++)
     {
         if(skill_name[a+startPoint] != "")
         {
             skillLabel[a]->setCaption(skill_name[a+startPoint]);
             skillLabel[a]->setVisible(true);
             char tmp[5];
             sprintf(tmp, "%d",playerSkill[a+startPoint].level);
             skillLevel[a]->setCaption(tmp);
             skillLevel[a]->setVisible(true);
             skillbar[a]->setProgress(0.0f);
             skillbar[a]->setVisible(true);
         }
         else
         {
             skillLevel[a]->setVisible(false);
             skillLabel[a]->setVisible(false);
             skillbar[a]->setVisible(false);
         }
     }
}
