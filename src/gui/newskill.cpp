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
<<<<<<< newskill.cpp
 *  $Id$
=======
 *  $Id$
>>>>>>> 1.2
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
    "Weaponsmithing", "Armorcrafting", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", ""
};

NewSkillDialog::NewSkillDialog():
    Window("Skills")
{
    // the window
    setSize(400, 300);

    // the skill labels
    skillLabel = new gcn::Label[N_SKILL];
    for (int a = 0; a < N_SKILL; a++) {
        skillLabel[a].setCaption(skill_name[a]);
        skillLabel[a].setDimension(gcn::Rectangle(8, 190, 200, 16));
    }

    // the close button
    closeButton = new Button("Close");
    closeButton->setEventId("close");
    closeButton->setPosition(160, 210);
    add(closeButton);

    // setting up the container
    skillList = new gcn::Container();
    skillList->setDimension(gcn::Rectangle(0,0,230,600));
    for(int b=0;b<N_SKILL;b++) {
        skillList->add(&skillLabel[b],20,6 + 12*b);}
        
    skillList->setDimension(gcn::Rectangle(0, 0, 230, 600));
    for (int b = 0; b < N_SKILL; b++) {
        skillList->add(&skillLabel[b], 20, 20 * b);
    }

    // the scroll area (content = container)
    skillScrollArea = new ScrollArea();
    skillScrollArea->setDimension(gcn::Rectangle(5, 5, 229, 290));
    skillScrollArea->setContent(skillList);
    add(skillScrollArea);




    closeButton->addActionListener(this);

    setLocationRelativeTo(getParent());
}

NewSkillDialog::~NewSkillDialog()
{
    delete skillScrollArea;
    delete []skillLabel;
    delete closeButton;
}

void NewSkillDialog::action(const std::string& eventId)
{
    if (eventId == "close")
    {
        setVisible(false);
    }
}
