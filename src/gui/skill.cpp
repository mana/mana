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

#include "skill.h"
#include "listbox.h"
#include "scrollarea.h"
#include "button.h"
#include "../main.h"

char *skill_db[] = {
    // 0-99
    /* weapons 0-9 */ "Short Blades", "Long Blades", "Hammers", "Archery", "Exotic", "Throwing", "Piercing", "Hand to Hand", "", "",
    /* magic 10-19 */ "Djin (Fire)", "Niksa (Water)", "Kerub (Earth)", "Ariel (Air)", "Paradin (Light)", "Tharsis (Dark)", "Crono (Time)", "Astra (Space)", "Gen (Mana)", "",
    /* craft 20-29 */ "Smithy", "Jeweler", "Alchemist", "Cook", "Tailor", "Artisan", "", "", "", "",
    /* general 30-39 */ "Running", "Searching", "Sneak", "Trading", "Intimidate", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    // 100-199
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
};

// pointer to the info struct the 
// dialog is concerned with (to insure future flexibility)
PLAYER_INFO** SkillList = &char_info; 


SkillDialog::SkillDialog():
    Window("Skills")
{
    /*skillListBox = new ListBox(this);
    skillScrollArea = new ScrollArea(skillListBox);
    pointsLabel = new gcn::Label("Skill Points:");
    incButton = new Button(" Up ");
    closeButton = new Button("Close");

    incButton->setEventId("inc");
    closeButton->setEventId("close");

    setSize(240, 240);
    skillScrollArea->setDimension(gcn::Rectangle(5, 5, 229, 180));
    pointsLabel->setDimension(gcn::Rectangle(8, 190, 200, 16));
    incButton->setPosition(64, 210);
    closeButton->setPosition(160, 210);

    add(skillScrollArea);
    add(pointsLabel);
    add(incButton);
    add(closeButton);

    incButton->addActionListener(this);
    closeButton->addActionListener(this);

    setLocationRelativeTo(getParent());*/

}

SkillDialog::~SkillDialog()
{
   /* delete skillListBox;
    delete skillScrollArea;
    delete pointsLabel;
    delete incButton;
    delete closeButton;*/

}

void SkillDialog::action(const std::string& eventId)
{
    // GUI design
}


