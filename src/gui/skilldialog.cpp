/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gui/skilldialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/container.h"
#include "gui/widgets/icon.h"
#include "gui/widgets/label.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/tabbedarea.h"
#include "gui/widgets/windowcontainer.h"

#include "localplayer.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <string>
#include <vector>

class SkillTab : public Container, public gcn::ActionListener
{
    public:
        /**
         * The type of this skill tab
         */
        const std::string type;

        /**
         * Constructor
         */
        SkillTab(const std::string &type);

        /**
         * Update this tab
         */
        void update();

        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event) {}

    private:
         /**
         * Update the information of a skill at
         * the given index
         */
        void updateSkill(int index);

        /**
         * Gets the number of skills in this particular
         * type of tab.
         */
        int getSkillNum();

        /**
         * Get the first enumeration of this skill tab's
         * skill type.
         */
        int getSkillBegin();

        /**
         * Get the icon associated with the given index
         */
        Icon *getIcon(int index);

        std::vector<Icon *> mSkillIcons;
        std::vector<gcn::Label *> mSkillNameLabels;
        std::vector<gcn::Label *> mSkillLevelLabels;
        std::vector<gcn::Label *> mSkillExpLabels;
        std::vector<ProgressBar *> mSkillProgress;
};


SkillDialog::SkillDialog():
    Window(_("Skills"))
{
    setWindowName("Skills");
    setCloseButton(true);
    setSaveVisible(true);
    setDefaultSize(windowContainer->getWidth() - 280, 30, 275, 425);

    TabbedArea *panel = new TabbedArea;
    panel->setDimension(gcn::Rectangle(5, 5, 270, 420));

    SkillTab *tab;

    // Add each type of skill tab to the panel
    tab = new SkillTab("Weapon");
    panel->addTab(_("Weapons"), tab);
    mTabs.push_back(tab);

    tab = new SkillTab("Magic");
    panel->addTab(_("Magic"), tab);
    mTabs.push_back(tab);

    tab = new SkillTab("Craft");
    panel->addTab(_("Crafts"), tab);
    mTabs.push_back(tab);

    add(panel);

    update();

    setLocationRelativeTo(getParent());
    loadWindowState();
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
  for(std::list<SkillTab*>::const_iterator i = mTabs.begin();
      i != mTabs.end(); ++i)
    {
      (*i)->update();
    }
}

SkillTab::SkillTab(const std::string &type): type(type)
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
    for (int a = 0; a < skillNum; a++)
    {
        mSkillIcons.at(a) = getIcon(a);
        mSkillIcons.at(a)->setPosition(1, a*32);
        add(mSkillIcons.at(a));

        mSkillNameLabels.at(a) = new Label("");
        mSkillNameLabels.at(a)->setPosition(35, a*32 );
        add(mSkillNameLabels.at(a));

        mSkillProgress.at(a) = new ProgressBar(0.0f, 200, 20, gcn::Color(150, 150, 150));
        mSkillProgress.at(a)->setPosition(35, a*32 + 13);
        add(mSkillProgress.at(a));

        mSkillExpLabels.at(a) = new Label("");
        mSkillExpLabels.at(a)->setPosition(45, a*32 + 16);
        add(mSkillExpLabels.at(a));

        mSkillLevelLabels.at(a) = new Label("");
        mSkillLevelLabels.at(a)->setPosition(165, a*32);
        add(mSkillLevelLabels.at(a));
    }

    update();
}

int SkillTab::getSkillNum()
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

int SkillTab::getSkillBegin()
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

Icon* SkillTab::getIcon(int index)
{
    int skillBegin = getSkillBegin();
    std::string icon = LocalPlayer::getSkillInfo(index + skillBegin).icon;
    return new Icon(icon);
}

void SkillTab::updateSkill(int index)
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

void SkillTab::update()
{
    int skillNum = getSkillNum();

    // Update the skill information for reach skill
    for (int a = 0; a < skillNum; a++)
    {
        updateSkill(a);
    }
}
