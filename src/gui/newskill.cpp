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
#include "listbox.h"
#include "scrollarea.h"
#include "button.h"
#include "../main.h"

char *skill_name[] = {
    // 0-99
    // weapon skills 0-9
    "Short Blades", "Long Blades", "Hammers", "Archery", "Whip",
    "Exotic", "Throwing ", "Piercing", "Hand to Hand", "",
    // magic skills 10-19
    "Djin (Fire)", "Niksa (Water)", "Earth (Kerub)", "Air (Ariel)",
    "Paradin (Light)", "Tharsis (Dark)", "Crono (Time)", "Astra (Space)",
    "Gen (Mana)", "",
    // craft skills 20-29
    "Weaponsmithing", "Armorcrafting", "Jeweler", "Cook", "Tailor",
    "Alchemist", "Artisan", "", "", "",
    // general skills 30-39
    "Running", "Jumping", "Searching", "Sneak", "Trading", "Intimidate",
    "", "", "", "",
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
    "Humanoids", "", "",
    // stats 80-89
    "Strength", "Fortitude", "Vitality", "Menality", "Awareness", "Mana",
    "Dexterity", "", "", "",
    // unused (reserved) 90-99
    "", "", "", "", "", "", "", "", "", ""
};

NewSkillDialog::NewSkillDialog():
    Window("Skills")
{
    
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
    setSize(560, 300);
    catButton[0]->setPosition(10, 10);
    catButton[1]->setPosition(60, 10);
    catButton[2]->setPosition(100, 10);
    catButton[3]->setPosition(180, 10);
    catButton[4]->setPosition(240, 10);
    catButton[5]->setPosition(300, 10);
    catButton[6]->setPosition(370, 10);
    catButton[7]->setPosition(430, 10);
    catButton[8]->setPosition(490, 10);
    closeButton->setPosition(480, 280);

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

    // setting up the container
    /*skillList = new gcn::Container();
    skillList->setOpaque(false);

    skillList->setDimension(gcn::Rectangle(0, 0, 230, 600));
    for (int b = 0; b < N_SKILL; b++) {
        skillList->add(&skillLabel[b], 20, 20 * b);
    }

    // the scroll area (content = container)
    skillScrollArea = new ScrollArea();
    skillScrollArea->setDimension(gcn::Rectangle(5, 5, 229, 290));
    skillScrollArea->setContent(skillList);
    add(skillScrollArea);*/



    // finsihing touches
    setLocationRelativeTo(getParent());
}

NewSkillDialog::~NewSkillDialog()
{
    delete skillbar;
    delete []skillLabel;
    delete closeButton;
    delete []catButton;
}

void NewSkillDialog::action(const std::string& eventId)
{
    if (eventId == "close")
    {
        setVisible(false);
    }
    else if (eventId == "g1") // weapons group 0-10
    {
        startPoint =0;
    }
}
