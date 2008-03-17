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

#include <algorithm>

#include <guichan/widgets/label.hpp>
#include <guichan/widgets/container.hpp>

#include "skill.h"

#include "button.h"
#include "listbox.h"
#include "scrollarea.h"
#include "windowcontainer.h"
#include "progressbar.h"
#include "tabbedcontainer.h"

#include "../localplayer.h"

#include "../utils/dtor.h"
#include "../utils/tostring.h"
#include "../utils/gettext.h"

SkillDialog::SkillDialog():
    Window(_("Skills"))
{
    setCloseButton(true);
    setDefaultSize(windowContainer->getWidth() - 255, 25, 230, 425);

    TabbedContainer *panel = new TabbedContainer();
    panel->setDimension(gcn::Rectangle(5, 5, 225, 420));
    panel->setOpaque(false);

    Skill_Tab* tab;

    tab = new Skill_Tab("Weapon");
    panel->addTab(tab, _("Weapons"));
    mTabs.push_back(tab);

    tab = new Skill_Tab("Magic");
    panel->addTab(tab, _("Magic"));
    mTabs.push_back(tab);

    tab = new Skill_Tab("Craft");
    panel->addTab(tab, _("Crafts"));
    mTabs.push_back(tab);

    add(panel);

    update();

    setLocationRelativeTo(getParent());
    loadWindowState(_("Skills"));
}

SkillDialog::~SkillDialog()
{
    for_each(mTabs.begin(), mTabs.end(), make_dtor(mTabs));
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
    for_each(mTabs.begin(), mTabs.end(), std::mem_fun(&Skill_Tab::update));
}

Skill_Tab::Skill_Tab(std::string type): type(type)
{
    int skillnum;
    if (type == "Weapon")
    {
        skillnum = CHAR_SKILL_WEAPON_NB;
    }
    else if (type == "Magic")
    {
        skillnum = CHAR_SKILL_MAGIC_NB;
    }
    else if (type == "Craft")
    {
        skillnum = CHAR_SKILL_CRAFT_NB;
    }

    mSkillNameLabels.resize(skillnum);
    mSkillLevelLabels.resize(skillnum);
    mSkillExpLabels.resize(skillnum);
    mSkillProgress.resize(skillnum);

    for (int a=0; a < skillnum; a++)
    {
        mSkillNameLabels.at(a) = new gcn::Label("");
        mSkillNameLabels.at(a)->setPosition(1, a*32 );
        add(mSkillNameLabels.at(a));

        mSkillProgress.at(a) = new ProgressBar(0.0f, 200, 20, 150, 150, 150);
        mSkillProgress.at(a)->setPosition(1, a*32 + 13);
        add(mSkillProgress.at(a));

        mSkillExpLabels.at(a) = new gcn::Label("");
        mSkillExpLabels.at(a)->setPosition(10, a*32 + 16);
        add(mSkillExpLabels.at(a));

        mSkillLevelLabels.at(a) = new gcn::Label("");
        mSkillLevelLabels.at(a)->setPosition(165, a*32);
        add(mSkillLevelLabels.at(a));
    }

    update();

}

void Skill_Tab::update()
{
    setOpaque(false);
    int skillnum;
    int skillbegin;
    if (type == "Weapon")
    {
        skillnum = CHAR_SKILL_WEAPON_NB;
        skillbegin = CHAR_SKILL_WEAPON_BEGIN - CHAR_SKILL_BEGIN;
    }
    else if (type == "Magic")
    {
        skillnum = CHAR_SKILL_MAGIC_NB;
        skillbegin = CHAR_SKILL_MAGIC_BEGIN - CHAR_SKILL_BEGIN;
    }
    else if (type == "Craft")
    {
        skillnum = CHAR_SKILL_CRAFT_NB;
        skillbegin = CHAR_SKILL_CRAFT_BEGIN - CHAR_SKILL_BEGIN;
    }

    for (int a = 0; a < skillnum; a++)
    {
        int baseLevel = player_node->getAttributeBase(a + skillbegin + CHAR_SKILL_BEGIN);
        int effLevel = player_node->getAttributeEffective(a + skillbegin + CHAR_SKILL_BEGIN);

        std::string skillLevel("Lvl: " + toString(baseLevel));
        if (effLevel < baseLevel)
        {
            skillLevel.append(" - " + toString(baseLevel - effLevel));
        }
        else if (effLevel > baseLevel)
        {
            skillLevel.append(" + " + toString(effLevel - baseLevel));
        }
        mSkillLevelLabels.at(a)->setCaption(skillLevel);

        std::pair<int, int> exp = player_node->getExperience(a + skillbegin);
        std::string sExp (toString(exp.first) + " / " + toString(exp.second));


        mSkillNameLabels.at(a)->setCaption(LocalPlayer::getSkillName(a + skillbegin));
        mSkillNameLabels.at(a)->adjustSize();
        mSkillLevelLabels.at(a)->adjustSize();
        mSkillExpLabels.at(a)->setCaption(sExp);
        mSkillExpLabels.at(a)->adjustSize();
        mSkillExpLabels.at(a)->setAlignment(gcn::Graphics::RIGHT);

        // more intense red as exp grows
        int color = 150 - (int)(150 * ((float) exp.first / exp.second));
        mSkillProgress.at(a)->setColor(244, color, color);
        mSkillProgress.at(a)->setProgress((float) exp.first / exp.second);
    }
}
