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

#include "gui/skill.h"

#include "gui/table.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/windowcontainer.h"

#include "localplayer.h"
#include "log.h"

#include "net/net.h"
#include "net/skillhandler.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/xml.h"

static const char *SKILLS_FILE = "skills.xml";

struct SkillInfo
{
    std::string name;
    bool modifiable;
};

static const SkillInfo fakeSkillInfo = {
    _("Mystery Skill"),
    false
};

std::vector<SkillInfo> skill_db;

static void initSkillinfo();

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

    virtual int getRows() const
    {
        return mEntriesNr;
    }

    virtual int getColumnWidth(int index) const
    {
        if (index == 0)
            return 160;

        return 35;
    }

    virtual int getRowHeight() const
    {
        return 12;
    }

    virtual void update()
    {
        mEntriesNr = mDialog->getSkills().size();
        resize();

        for (int i = 0; i < mEntriesNr; i++)
        {
            SKILL *skill = mDialog->getSkills()[i];
            SkillInfo const *info;
            char tmp[128];

            if (skill->id >= 0
                && (unsigned int) skill->id < skill_db.size())
                info = &skill_db[skill->id];
            else
                info = &fakeSkillInfo;

            sprintf(tmp, "%c%s", info->modifiable? ' ' : '*', info->name.c_str());
            gcn::Label *name_label = new Label(tmp);

            sprintf(tmp, "Lv:%i", skill->lv);
            gcn::Label *lv_label = new Label(tmp);

            sprintf(tmp, "Sp:%i", skill->sp);
            gcn::Label *sp_label = new Label(tmp);

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
    Window(_("Skills"))
{
    initSkillinfo();
    mTableModel = new SkillGuiTableModel(this);
    mTable = new GuiTable(mTableModel);
    mTable->setOpaque(false);
    mTable->setLinewiseSelection(true);
    mTable->setWrappingEnabled(true);
    mTable->setActionEventId("skill");
    mTable->addActionListener(this);

    setWindowName("Skills");
    setCloseButton(true);
    setDefaultSize(255, 260, ImageRect::CENTER);

    setMinHeight(50 + mTableModel->getHeight());
    setMinWidth(200);

    ScrollArea *skillScrollArea = new ScrollArea(mTable);
    mPointsLabel = new Label(strprintf(_("Skill points: %d"), 0));
    mIncButton = new Button(_("Up"), "inc", this);
    mUseButton = new Button(_("Use"), "use", this);
    mUseButton->setEnabled(false);

    skillScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    place(0, 0, skillScrollArea, 5).setPadding(3);
    place(0, 1, mPointsLabel, 4);
    place(3, 2, mIncButton);
    place(4, 2, mUseButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    center();
    loadWindowState();
}

SkillDialog::~SkillDialog()
{
    delete_all(mSkillList);
}

void SkillDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "inc")
    {
        // Increment skill
        int selectedSkill = mTable->getSelectedRow();
        if (selectedSkill >= 0)
            Net::getSkillHandler()->up(mSkillList[selectedSkill]->id);
    }
    else if (event.getId() == "skill" && mTable->getSelectedRow() > -1)
    {
        SKILL *skill = mSkillList[mTable->getSelectedRow()];
        SkillInfo const *info;

        if (skill->id >= 0 && (unsigned int) skill->id < skill_db.size())
            info = &skill_db[skill->id];
        else
            info = &fakeSkillInfo;

        mIncButton->setEnabled(player_node->mSkillPoint > 0 &&
                               info->modifiable);
    }
    else if (event.getId() == "close")
        setVisible(false);
}

void SkillDialog::update()
{
    mPointsLabel->setCaption(strprintf(_("Skill points: %d"),
                                       player_node->mSkillPoint));

    int selectedSkill = mTable->getSelectedRow();

    if (selectedSkill >= 0)
    {
        int skillId = mSkillList[selectedSkill]->id;
        bool modifiable;

        if (skillId >= 0 && (unsigned int) skillId < skill_db.size())
            modifiable = skill_db[skillId].modifiable;
        else
            modifiable = false;

        mIncButton->setEnabled(modifiable
                               && player_node->mSkillPoint > 0);
    }
    else
        mIncButton->setEnabled(false);

    mTableModel->update();
    setMinHeight(50 + mTableModel->getHeight());
}

int SkillDialog::getNumberOfElements()
{
    return mSkillList.size();
}

bool SkillDialog::hasSkill(int id)
{
    for (unsigned int i = 0; i < mSkillList.size(); i++)
    {
        if (mSkillList[i]->id == id)
            return true;
    }
    return false;
}

void SkillDialog::addSkill(int id, int lvl, int mp)
{
    SKILL *tmp = new SKILL;
    tmp->id = id;
    tmp->lv = lvl;
    tmp->sp = mp;
    mSkillList.push_back(tmp);
}

void SkillDialog::setSkill(int id, int lvl, int mp)
{
    for (unsigned int i = 0; i < mSkillList.size(); i++)
    {
        if (mSkillList[i]->id == id)
        {
            mSkillList[i]->lv = lvl;
            mSkillList[i]->sp = mp;
        }
    }
}

void SkillDialog::cleanList()
{
    delete_all(mSkillList);
    mSkillList.clear();
}

static void initSkillinfo()
{
    SkillInfo emptySkillInfo = { "", false };

    XML::Document doc(SKILLS_FILE);
    xmlNodePtr root = doc.rootNode();

    if (!root || !xmlStrEqual(root->name, BAD_CAST "skills"))
    {
        logger->log("Error loading skills file: %s", SKILLS_FILE);
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

            if (index >= 0)
            {
                skill_db.resize(index + 1, emptySkillInfo);
                skill_db[index].name = name;
                skill_db[index].modifiable = modifiable;
            }
        }
    }
}

