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

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "listbox.h"
#include "scrollarea.h"

#include "../localplayer.h"
#include "../graphics.h"

#include "../utils/dtor.h"

extern Graphics *graphics;

const char *skill_db[] = {
    // 0-99
    "", "Basic", "Sword", "Two hand", "HP regeneration", "Bash", "Provoke", "Magnum", "Endure", "MP regeneration",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "MAX weight", "Discount", "Overcharge", "",
    "Identify", "", "", "", "", "", "", "", "Double", "Miss",
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
    "", "", "First aid", "Play as dead", "Moving recovery", "Fatal blow", "Auto berserk", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
};


SkillDialog::SkillDialog():
    Window("Skills")
{
    setWindowName("Skills");
    setDefaultSize(graphics->getWidth() - 255, 25, 240, 240);

    skillListBox = new ListBox(this);
    skillScrollArea = new ScrollArea(skillListBox);
    pointsLabel = new gcn::Label("Skill Points:");
    incButton = new Button("Up", "inc", this);
    useButton = new Button("Use", "use", this);
    useButton->setEnabled(false);
    closeButton = new Button("Close", "close", this);

    skillListBox->setEventId("skill");

    skillScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    skillScrollArea->setDimension(gcn::Rectangle(5, 5, 230, 180));
    pointsLabel->setDimension(gcn::Rectangle(8, 190, 200, 16));
    incButton->setPosition(skillScrollArea->getX(), 210);
    useButton->setPosition(incButton->getX() + incButton->getWidth() + 5,
        210);
    closeButton->setPosition(
        skillScrollArea->getX() + skillScrollArea->getWidth() - closeButton->getWidth(),
        210);

    add(skillScrollArea);
    add(pointsLabel);
    add(incButton);
    add(useButton);
    add(closeButton);

    skillListBox->addActionListener(this);

    setLocationRelativeTo(getParent());
    loadWindowState();
}

SkillDialog::~SkillDialog()
{
    cleanList();
}

void SkillDialog::action(const std::string& eventId)
{
    if (eventId == "inc")
    {
        // Increment skill
        int selectedSkill = skillListBox->getSelected();
        if (selectedSkill >= 0)
        {
            player_node->raiseSkill(skillList[selectedSkill]->id);
        }
    }
    else if (eventId == "skill")
    {
        incButton->setEnabled(
                skillListBox->getSelected() > -1 &&
                player_node->skillPoint > 0);
    }
    else if (eventId == "close")
    {
        setVisible(false);
    }
}

void SkillDialog::update()
{
    if (pointsLabel != NULL) {
        char tmp[128];
        sprintf(tmp, "Skill points: %i", player_node->skillPoint);
        pointsLabel->setCaption(tmp);
    }

    incButton->setEnabled(skillListBox->getSelected() > -1 && player_node->skillPoint > 0);
}

int SkillDialog::getNumberOfElements()
{
    return skillList.size();
}

std::string SkillDialog::getElementAt(int i)
{
    if (i >= 0 && i < (int)skillList.size())
    {
        char tmp[128];
        sprintf(tmp, "%s    Lv: %i    Sp: %i",
                skill_db[skillList[i]->id],
                skillList[i]->lv,
                skillList[i]->sp);
        return tmp;
    }
    return "";
}

bool SkillDialog::hasSkill(int id)
{
    for (unsigned int i = 0; i < skillList.size(); i++) {
        if (skillList[i]->id == id) {
            return true;
        }
    }
    return false;
}

void SkillDialog::addSkill(int id, int lvl, int mp)
{
    SKILL *tmp = new SKILL();
    tmp->id = id;
    tmp->lv = lvl;
    tmp->sp = mp;
    skillList.push_back(tmp);
}

void SkillDialog::setSkill(int id, int lvl, int mp)
{
    for (unsigned int i = 0; i < skillList.size(); i++) {
        if (skillList[i]->id == id) {
            skillList[i]->lv = lvl;
            skillList[i]->sp = mp;
        }
    }
}

void SkillDialog::cleanList()
{
    for_each(skillList.begin(), skillList.end(), make_dtor(skillList));
    skillList.clear();
}
