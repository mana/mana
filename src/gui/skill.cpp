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

extern PLAYER_INFO  *char_info;

char *skill_db[] = {
	// 0-99
	"", "Basic", "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
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
	"", "", "First aid", "Play as dead", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
};


SkillListModel::SkillListModel()
{
}

SkillListModel::~SkillListModel()
{
    for (int i = skillList.size() - 1; i >= 0; i--)
    {
        delete skillList[i];
        skillList.pop_back();
    }
}

int SkillListModel::getNumberOfElements()
{
    return skillList.size();
}

std::string SkillListModel::getElementAt(int i)
{
    if (i >= 0 && i < (int)skillList.size())
    {
        //return skill_db[skillList[i]->id];
        char tmp[128];
        sprintf(tmp, "%s    Lv: %i    Sp: %i", skill_db[skillList[i]->id], skillList[i]->lv, skillList[i]->sp);
        return tmp;
    }
    return "";
}

bool SkillListModel::hasSkill(int id)
{
    for (unsigned int i = 0; i < skillList.size(); i++) {
        if (skillList[i]->id == id) {
            return true;
        }
    }
    return false;
}

void SkillListModel::addSkill(int id, int lv, int sp)
{
    SKILL *tmp = new SKILL;
    tmp->id = id;
    tmp->lv = lv;
    tmp->sp = sp;
    skillList.push_back(tmp);
}

void SkillListModel::setSkill(int id, int lv, int sp)
{
    if (!hasSkill(id)) {
        return;
    }
    skillList[id]->lv = lv;
    skillList[id]->sp = sp;
}


SkillDialog::SkillDialog(gcn::Container *parent)
    : Window(parent, "Skills")
{
    skills = new SkillListModel();
    skillListBox = new ListBox(skills);
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

    setLocationRelativeTo(getParent());
}

SkillDialog::~SkillDialog()
{
    delete skillListBox;
    delete skillScrollArea;
    delete pointsLabel;
    delete incButton;
    delete closeButton;

    delete skills;
}

void SkillDialog::action(const std::string& eventId)
{
    if (eventId == "inc")
    {
        //increment skill
    }
    else if (eventId == "close")
    {
        setVisible(false);
    }
}

void SkillDialog::setPoints(int i)
{
    char tmp[128];
    sprintf(tmp, "Skill points: %i", i);
    if (pointsLabel != NULL) {
        pointsLabel->setCaption(tmp);
    }
}

