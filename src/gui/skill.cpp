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

#include "skill.h"

#include "button.h"
#include "listbox.h"
#include "windowcontainer.h"

#include "../localplayer.h"

#include "../utils/dtor.h"
#include "../utils/xml.h"
#include "log.h"

#define SKILLS_FILE "skills.xml"

struct SkillInfo {
    std::string name;
    bool modifiable;
};

std::vector<SkillInfo> skill_db;

static void initSkillinfo(void);

class SkillGuiTableModel : public StaticTableModel
{
public:
    SkillGuiTableModel(SkillDialog *dialog) :
        StaticTableModel(0, 3)
    {
        mEntriesNr = 0;
        mDialog = dialog;
        update();
    }

    virtual int
    getRows(void) { return mEntriesNr; }

    virtual int
    getColumnWidth(int index)
    {
        switch (index) {
        case 0:  return 160;
        default: return 35;
        }
    }

    virtual int
    getRowHeight(void)
    {
        return 12;
    }

    virtual void
    update(void)
    {
	static const SkillInfo fakeSkillInfo = { "???", false };

        mEntriesNr = mDialog->getSkills().size();
        resize();

        for (int i = 0; i < mEntriesNr; i++) {
            SKILL *skill = mDialog->getSkills()[i];
            SkillInfo const *info;
            char tmp[128];

            if (skill->id >= 0 && (unsigned int) skill->id < skill_db.size())
                info = &skill_db[skill->id];
            else
                info = &fakeSkillInfo;

            sprintf(tmp, "%c%s", info->modifiable? ' ' : '*', info->name.c_str());
            gcn::Label *name_label = new gcn::Label(std::string(tmp));

            sprintf(tmp, "Lv:%i", skill->lv);
            gcn::Label *lv_label = new gcn::Label(std::string(tmp));

            sprintf(tmp, "Sp:%i", skill->sp);
            gcn::Label *sp_label = new gcn::Label(std::string(tmp));

            set(i, 0, name_label);
            set(i, 1, lv_label);
            set(i, 2, sp_label);
        }
    }


private:
    SkillDialog *mDialog;
    int mEntriesNr;
};


SkillDialog::SkillDialog():
    Window("Skills")
{
    initSkillinfo();
    mTableModel = new SkillGuiTableModel(this);
    mTable.setModel(mTableModel);
    mTable.setLinewiseSelection(true);

    setWindowName("Skills");
    setCloseButton(true);
    setDefaultSize(windowContainer->getWidth() - 255, 25, 240, 240);

//    mSkillListBox = new ListBox(this);
    skillScrollArea = new ScrollArea(&mTable);
    mPointsLabel = new gcn::Label("Skill Points:");
    mIncButton = new Button("Up", "inc", this);
    mUseButton = new Button("Use", "use", this);
    mUseButton->setEnabled(false);

//    mSkillListBox->setActionEventId("skill");
    mTable.setActionEventId("skill");

    skillScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    skillScrollArea->setDimension(gcn::Rectangle(5, 5, 230, 180));
    mPointsLabel->setDimension(gcn::Rectangle(8, 190, 200, 16));
    mIncButton->setPosition(skillScrollArea->getX(), 210);
    mUseButton->setPosition(mIncButton->getX() + mIncButton->getWidth() + 5,
        210);

    add(skillScrollArea);
    add(mPointsLabel);
    add(mIncButton);
    add(mUseButton);

//    mSkillListBox->addActionListener(this);
    mTable.addActionListener(this);

    setLocationRelativeTo(getParent());
    loadWindowState();
}

SkillDialog::~SkillDialog()
{
}

void SkillDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "inc")
    {
        // Increment skill
        int selectedSkill = mTable.getSelectedRow();//mSkillListBox->getSelected();
        if (selectedSkill >= 0)
        {
            player_node->raiseSkill(mSkillList[selectedSkill]->id);
        }
    }
    else if (event.getId() == "skill")
    {
        mIncButton->setEnabled(
                mTable.getSelectedRow() > -1 &&
                player_node->mSkillPoint > 0);
    }
    else if (event.getId() == "close")
    {
        setVisible(false);
    }
}

void SkillDialog::update()
{
    if (mPointsLabel != NULL) {
        char tmp[128];
        sprintf(tmp, "Skill points: %i", player_node->mSkillPoint);
        mPointsLabel->setCaption(tmp);
    }

    int selectedSkill = mTable.getSelectedRow();

    if (selectedSkill >= 0) {
        int skillId = mSkillList[selectedSkill]->id;
        bool modifiable;
 	 
        if (skillId >= 0 && (unsigned int) skillId < skill_db.size())
            modifiable = skill_db[skillId].modifiable;
        else
            modifiable = false;

        mIncButton->setEnabled(modifiable && player_node->mSkillPoint > 0);
    } else
        mIncButton->setEnabled(false);

    mTableModel->update();
}

int SkillDialog::getNumberOfElements()
{
    return mSkillList.size();
}

bool SkillDialog::hasSkill(int id)
{
    for (unsigned int i = 0; i < mSkillList.size(); i++) {
        if (mSkillList[i]->id == id) {
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
    mSkillList.push_back(tmp);
}

void SkillDialog::setSkill(int id, int lvl, int mp)
{
    for (unsigned int i = 0; i < mSkillList.size(); i++) {
        if (mSkillList[i]->id == id) {
            mSkillList[i]->lv = lvl;
            mSkillList[i]->sp = mp;
        }
    }
}

void SkillDialog::cleanList()
{
    for_each(mSkillList.begin(), mSkillList.end(), make_dtor(mSkillList));
    mSkillList.clear();
}

static void
initSkillinfo(void)
{
    SkillInfo emptySkillInfo = { "", false };

    XML::Document doc(SKILLS_FILE);
    xmlNodePtr root = doc.rootNode();

    if (!root || !xmlStrEqual(root->name, BAD_CAST "skills"))
    {
        logger->log("Error loading skills file: "
                    SKILLS_FILE);
        skill_db.resize(2, emptySkillInfo);
        skill_db[1].name = "Basic";
        skill_db[1].modifiable = true;
        return;
    }

    for_each_xml_child_node(node, root)
    {
        if (xmlStrEqual(node->name, BAD_CAST "skill"))
        {
            int index = atoi(XML::getProperty(node, "id", "-1").c_str());
            std::string name = XML::getProperty(node, "name", "");
            bool modifiable = !atoi(XML::getProperty(node, "fixed", "0").c_str());

            if (index >= 0) {
                skill_db.resize(index + 1, emptySkillInfo);
                skill_db[index].name = name;
                skill_db[index].modifiable = modifiable;
            }
        }
    }
}

