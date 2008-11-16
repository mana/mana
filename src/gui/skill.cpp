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

#include <guichan/widgets/label.hpp>
#include <guichan/widgets/container.hpp>
#include <guichan/widgets/icon.hpp>

#include "skill.h"

#include "icon.h"
#include "button.h"
#include "listbox.h"
#include "scrollarea.h"
#include "windowcontainer.h"
#include "progressbar.h"

#include "widgets/tabbedarea.h"

#include "../localplayer.h"

#include "../utils/dtor.h"
#include "../utils/tostring.h"
#include "../utils/gettext.h"

SkillDialog::SkillDialog():
    Window(_("Skills"))
{
    setCloseButton(true);
    setDefaultSize(windowContainer->getWidth() - 255, 25, 275, 425);

    TabbedArea *panel = new TabbedArea();
    panel->setDimension(gcn::Rectangle(5, 5, 270, 420));

    Skill_Tab* tab;

    // Add each type of skill tab to the panel
    tab = new Skill_Tab("Weapon");
    panel->addTab(_("Weapons"), tab);
    mTabs.push_back(tab);

    tab = new Skill_Tab("Magic");
    panel->addTab(_("Magic"), tab);
    mTabs.push_back(tab);

    tab = new Skill_Tab("Craft");
    panel->addTab(_("Crafts"), tab);
    mTabs.push_back(tab);

    add(panel);

    update();

    setLocationRelativeTo(getParent());
    loadWindowState(_("Skills"));
}

SkillDialog::~SkillDialog()
{
    delete_all(mTabs);
}

void SkillDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "skill")
    {
    }
    else if (event.getId() == "close")
    {
        setVisible(false);
    }
}

void SkillDialog::draw(gcn::Graphics *g)
{
    update();

    Window::draw(g);
}

void SkillDialog::update()
{
  for(std::list<Skill_Tab*>::const_iterator i = mTabs.begin();
      i != mTabs.end(); ++i)
    {
      (*i)->update();
    }
}

Skill_Tab::Skill_Tab(const std::string &type): type(type)
{
    setOpaque(false);
    setDimension(gcn::Rectangle(0, 0, 270, 420));
    int skillNum = getSkillNum();

    mSkillIcons.resize(skillNum);
    mSkillNameLabels.resize(skillNum);
    mSkillLevelLabels.resize(skillNum);
    mSkillExpLabels.resize(skillNum);
    mSkillProgress.resize(skillNum);

    // Set the initial positions of the skill information
    for (int a=0; a < skillNum; a++)
    {
        mSkillIcons.at(a) = getIcon(a);
        mSkillIcons.at(a)->setPosition(1, a*32);
        add(mSkillIcons.at(a));

        mSkillNameLabels.at(a) = new gcn::Label("");
        mSkillNameLabels.at(a)->setPosition(35, a*32 );
        add(mSkillNameLabels.at(a));

        mSkillProgress.at(a) = new ProgressBar(0.0f, 200, 20, 150, 150, 150);
        mSkillProgress.at(a)->setPosition(35, a*32 + 13);
        add(mSkillProgress.at(a));

        mSkillExpLabels.at(a) = new gcn::Label("");
        mSkillExpLabels.at(a)->setPosition(45, a*32 + 16);
        add(mSkillExpLabels.at(a));

        mSkillLevelLabels.at(a) = new gcn::Label("");
        mSkillLevelLabels.at(a)->setPosition(165, a*32);
        add(mSkillLevelLabels.at(a));
    }

    update();

}

int Skill_Tab::getSkillNum()
{
    int skillNum = 0;

    if (type == "Weapon")
    {
        skillNum = CHAR_SKILL_WEAPON_NB;
        return skillNum;
    }
    else if (type == "Magic")
    {
        skillNum = CHAR_SKILL_MAGIC_NB;
        return skillNum;
    }
    else if (type == "Craft")
    {
        skillNum = CHAR_SKILL_CRAFT_NB;
        return skillNum;
    }
    else return skillNum;
}

int Skill_Tab::getSkillBegin()
{
    int skillBegin = 0;

    if (type == "Weapon")
    {
        skillBegin = CHAR_SKILL_WEAPON_BEGIN - CHAR_SKILL_BEGIN;
        return skillBegin;
    }
    else if (type == "Magic")
    {
        skillBegin = CHAR_SKILL_MAGIC_BEGIN - CHAR_SKILL_BEGIN;
        return skillBegin;
    }
    else if (type == "Craft")
    {
        skillBegin = CHAR_SKILL_CRAFT_BEGIN - CHAR_SKILL_BEGIN;
        return skillBegin;
    }
    else return skillBegin;
}

Icon* Skill_Tab::getIcon(int index)
{
    int skillBegin = getSkillBegin();
    std::string icon = LocalPlayer::getSkillInfo(index + skillBegin).icon;
    return new Icon(icon);
}

void Skill_Tab::updateSkill(int index)
{
    int skillBegin = getSkillBegin();

    int baseLevel = player_node->getAttributeBase(index +
                                                  skillBegin +
                                                  CHAR_SKILL_BEGIN);

    int effLevel = player_node->getAttributeEffective(index +
                                                      skillBegin +
                                                      CHAR_SKILL_BEGIN);
    if(baseLevel <= 0)
    {
        mSkillProgress.at(index)->setVisible(false);
        mSkillExpLabels.at(index)->setVisible(false);
        mSkillLevelLabels.at(index)->setVisible(false);
        mSkillNameLabels.at(index)->setVisible(false);
        mSkillIcons.at(index)->setVisible(false);
    }
    else
    {
        mSkillProgress.at(index)->setVisible(true);
        mSkillExpLabels.at(index)->setVisible(true);
        mSkillLevelLabels.at(index)->setVisible(true);
        mSkillNameLabels.at(index)->setVisible(true);
        mSkillIcons.at(index)->setVisible(true);
        std::string skillLevel("Lvl: " + toString(baseLevel));
        if (effLevel < baseLevel)
        {
            skillLevel.append(" - " + toString(baseLevel - effLevel));
        }
        else if (effLevel > baseLevel)
        {
            skillLevel.append(" + " + toString(effLevel - baseLevel));
        }
        mSkillLevelLabels.at(index)->setCaption(skillLevel);

        std::pair<int, int> exp = player_node->getExperience(index + skillBegin);
        std::string sExp (toString(exp.first) + " / " + toString(exp.second));


        mSkillNameLabels.at(index)->setCaption(LocalPlayer::getSkillInfo(index + skillBegin).name);
        mSkillNameLabels.at(index)->adjustSize();
        mSkillLevelLabels.at(index)->adjustSize();
        mSkillExpLabels.at(index)->setCaption(sExp);
        mSkillExpLabels.at(index)->adjustSize();
        mSkillExpLabels.at(index)->setAlignment(gcn::Graphics::RIGHT);

        // More intense red as exp grows
        int color = 150 - (int)(150 * ((float) exp.first / exp.second));
        mSkillProgress.at(index)->setColor(244, color, color);
        mSkillProgress.at(index)->setProgress((float) exp.first / exp.second);
    }
}

void Skill_Tab::update()
{
    int skillNum = getSkillNum();

    // Update the skill information for reach skill
    for (int a = 0; a < skillNum; a++)
    {
        updateSkill(a);
    }
}
