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

#include "../playerinfo.h"

#include "../net/network.h"

char *skill_db[] = {
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
    Window("Skills"),
    skillPoints(0)
{
    skillListBox = new ListBox(this);
    skillScrollArea = new ScrollArea(skillListBox);
    pointsLabel = new gcn::Label("Skill Points:");
    incButton = new Button("Up");
    useButton = new Button("Use");
    useButton->setEnabled(false);
    closeButton = new Button("Close");

    skillListBox->setEventId("skill");
    incButton->setEventId("inc");
    useButton->setEventId("use");
    closeButton->setEventId("close");

    setContentSize(240, 240);
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
    incButton->addActionListener(this);
    useButton->addActionListener(this);
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

    for (int i = skillList.size() - 1; i >= 0; i--)
    {
        delete skillList[i];
        skillList.pop_back();
    }
}

void SkillDialog::action(const std::string& eventId)
{
    if (eventId == "inc")
    {
        // Increment skill
        int selectedSkill = skillListBox->getSelected();
        if (char_info->skill_point > 0 && selectedSkill >= 0)
        {
            WFIFOW(0) = net_w_value(0x0112);
            WFIFOW(2) = net_w_value(
                    skillList[selectedSkill]->id);
            WFIFOSET(4);
        }
    }
    else if (eventId == "skill")
    {
        incButton->setEnabled(
                skillListBox->getSelected() > -1 &&
                skillPoints > 0);
    }
    else if (eventId == "close")
    {
        setVisible(false);
    }
}

void SkillDialog::setPoints(int i)
{
    skillPoints = i;

    if (pointsLabel != NULL) {
        char tmp[128];
        sprintf(tmp, "Skill points: %i", skillPoints);
        pointsLabel->setCaption(tmp);
    }

    incButton->setEnabled(skillListBox->getSelected() > -1 && skillPoints > 0);
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

void SkillDialog::addSkill(int id, int lv, int sp)
{
    printf("%i\n", id);
    SKILL *tmp = new SKILL();
    tmp->id = id;
    tmp->lv = lv;
    tmp->sp = sp;
    skillList.push_back(tmp);
}

void SkillDialog::setSkill(int id, int lv, int sp)
{
    for (unsigned int i = 0; i < skillList.size(); i++) {
        if (skillList[i]->id == id) {
            skillList[i]->lv = lv;
            skillList[i]->sp = sp;
        }
    }
}

void SkillDialog::cleanList()
{
    for (int i = skillList.size() - 1; i >= 0; i--)
    {
        delete skillList[i];
        skillList.pop_back();
    }
}
