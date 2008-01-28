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

#include "../localplayer.h"

#include "../utils/dtor.h"
#include "../utils/toString.h"
#include "../utils/gettext.h"

SkillDialog::SkillDialog():
    Window("Skills")
{
    setCloseButton(true);
    setDefaultSize(windowContainer->getWidth() - 255, 25, 240, 240);

    mSkillNameLabels.resize(CHAR_SKILL_NB);
    mSkillLevelLabels.resize(CHAR_SKILL_NB);
    mSkillExpLabels.resize(CHAR_SKILL_NB);

    for (int a=0; a < CHAR_SKILL_NB; a++)
    {
        mSkillNameLabels.at(a) = new gcn::Label("");
        mSkillNameLabels.at(a)->setPosition(1, a*10);
        add(mSkillNameLabels.at(a));
        mSkillLevelLabels.at(a) = new gcn::Label("");
        mSkillLevelLabels.at(a)->setPosition(75, a*10);
        add(mSkillLevelLabels.at(a));
        mSkillExpLabels.at(a) = new gcn::Label("");
        mSkillExpLabels.at(a)->setPosition(150, a*10);
        add(mSkillExpLabels.at(a));
    }

    update();

    setLocationRelativeTo(getParent());
    loadWindowState(_("Skills"));
}

SkillDialog::~SkillDialog()
{

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
    for (int a = 0; a < CHAR_SKILL_NB; a++)
    {
        int baseLevel = player_node->getAttributeBase(a + CHAR_SKILL_BEGIN);
        int effLevel = player_node->getAttributeEffective(a + CHAR_SKILL_BEGIN);
        std::string skillLevel("Lvl:" + toString(effLevel) + " / " + toString(baseLevel));

        std::pair<int, int> exp = player_node->getExperience(a);
        std::string sExp (toString(exp.first) + " / " + toString(exp.second));

        mSkillNameLabels.at(a)->setCaption("Skill" + toString(a));
        mSkillNameLabels.at(a)->adjustSize();
        mSkillLevelLabels.at(a)->setCaption(skillLevel);
        mSkillLevelLabels.at(a)->adjustSize();
        mSkillExpLabels.at(a)->setCaption(sExp);
        mSkillExpLabels.at(a)->adjustSize();
    }
}
