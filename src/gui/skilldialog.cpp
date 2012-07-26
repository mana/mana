/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/skilldialog.h"

#include "log.h"
#include "playerinfo.h"
#include "configuration.h"
#include "eventlistener.h"

#include "gui/setup.h"

#include "gui/widgets/button.h"
#include "gui/widgets/container.h"
#include "gui/widgets/label.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/tab.h"
#include "gui/widgets/tabbedarea.h"
#include "gui/widgets/windowcontainer.h"

#include "net/net.h"
#include "net/playerhandler.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"
#include "resources/theme.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/xml.h"

#include <guichan/font.hpp>

#include <set>
#include <string>

#define SKILLS_FILE "skills.xml"

class SkillModel;
class SkillEntry;

struct SkillInfo
{
    unsigned short id;
    std::string name;
    Image *icon;
    bool modifiable;
    bool visible;
    SkillModel *model;

    std::string skillLevel;
    int skillLevelWidth;

    std::string skillExp;
    float progress;
    gcn::Color color;

    SkillInfo() :
        icon(NULL)
    {}

    ~SkillInfo()
    {
        if (icon)
            icon->decRef();
    }

    void setIcon(const std::string &iconPath)
    {
        ResourceManager *res = ResourceManager::getInstance();
        if (!iconPath.empty())
        {
            icon = res->getImage(iconPath);
        }

        if (!icon)
        {
            icon = Theme::getImageFromTheme(
                                       paths.getStringValue("unknownItemFile"));
        }
    }

    void update();

    void draw(Graphics *graphics, int y, int width);
};

typedef std::vector<SkillInfo*> SkillList;

class SkillModel : public gcn::ListModel
{
public:
    int getNumberOfElements()
    { return mVisibleSkills.size(); }

    SkillInfo *getSkillAt(int i) const
    { return mVisibleSkills.at(i); }

    std::string getElementAt(int i)
    { return getSkillAt(i)->name; }

    void updateVisibilities();

    void addSkill(SkillInfo *info)
    { mSkills.push_back(info); }

private:
    SkillList mSkills;
    SkillList mVisibleSkills;
};

class SkillListBox : public ListBox
{
public:
    SkillListBox(SkillModel *model):
            ListBox(model)
    {}

    SkillInfo *getSelectedInfo()
    {
        const int selected = getSelected();
        if (selected < 0 || selected > mListModel->getNumberOfElements())
            return 0;

        return static_cast<SkillModel*>(mListModel)->getSkillAt(selected);
    }

    void draw(gcn::Graphics *gcnGraphics)
    {
        if (!mListModel)
            return;

        SkillModel* model = static_cast<SkillModel*>(mListModel);

        updateAlpha();

        Graphics *graphics = static_cast<Graphics*>(gcnGraphics);

        graphics->setColor(Theme::getThemeColor(Theme::HIGHLIGHT,
                                                (int) (mAlpha * 255.0f)));
        graphics->setFont(getFont());

        // Draw filled rectangle around the selected list element
        if (mSelected >= 0)
        {
            graphics->fillRectangle(gcn::Rectangle(0, getRowHeight() * mSelected,
                                                   getWidth(), getRowHeight()));
        }

        // Draw the list elements
        graphics->setColor(Theme::getThemeColor(Theme::TEXT));
        for (int i = 0, y = 1;
             i < model->getNumberOfElements();
             ++i, y += getRowHeight())
        {
            SkillInfo *e = model->getSkillAt(i);

            if (e)
            {
                e->draw(graphics, y, getWidth());
            }
        }
    }

    unsigned int getRowHeight() const { return 34; }
};

class SkillTab : public Tab
{
public:
    SkillTab(const std::string &name, SkillListBox *listBox):
            mListBox(listBox)
    {
        setCaption(name);
    }

    ~SkillTab()
    {
        delete mListBox;
        mListBox = 0;
    }

    SkillInfo *getSelectedInfo()
    {
        return mListBox->getSelectedInfo();
    }

private:
    SkillListBox *mListBox;
};

SkillDialog::SkillDialog():
    Window(_("Skills"))
{
    listen(Event::AttributesChannel);

    setWindowName("Skills");
    setCloseButton(true);
    setResizable(true);
    setSaveVisible(true);
    setDefaultSize(windowContainer->getWidth() - 280, 30, 275, 425);
    setMinHeight(113);
    setMinWidth(240);
    setupWindow->registerWindowForReset(this);

    mTabs = new TabbedArea();

    place(0, 0, mTabs, 5, 5);

    center();
    loadWindowState();
}

SkillDialog::~SkillDialog()
{
    clearSkills();
}

void SkillDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "close")
        setVisible(false);
}

std::string SkillDialog::update(int id)
{
    SkillMap::iterator i = mSkills.find(id);

    if (i != mSkills.end())
    {
        SkillInfo *info = i->second;
        info->update();
        return info->name;
    }

    return std::string();
}

void SkillDialog::update()
{
    for (SkillMap::iterator it = mSkills.begin(); it != mSkills.end(); it++)
    {
        (*it).second->update();
    }
}

void SkillDialog::event(Event::Channel channel, const Event &event)
{
    if (event.getType() == Event::UpdateAttribute)
    {
        if (event.getInt("id") == SKILL_POINTS)
        {
            update();
        }
    }
    else if (event.getType() == Event::UpdateStat)
    {
        SkillMap::iterator it = mSkills.find(event.getInt("id"));
        if (it != mSkills.end())
            it->second->update();
    }
}

void SkillDialog::clearSkills()
{
    // Fixes issues with removing tabs
    if (mTabs->getSelectedTabIndex() != -1)
    {
        mTabs->setSelectedTab((unsigned int) 0);

        while (mTabs->getSelectedTabIndex() != -1)
        {
            gcn::Tab *tab = mTabs->getSelectedTab();
            mTabs->removeTabWithIndex(mTabs->getSelectedTabIndex());
            delete tab;
        }
    }

    delete_all(mSkills);
    mSkills.clear();
}

void SkillDialog::loadSkills()
{
    clearSkills();

    XML::Document doc(SKILLS_FILE);
    xmlNodePtr root = doc.rootNode();

    int setCount = 0;
    std::string setName;
    ScrollArea *scroll;
    SkillListBox *listbox;
    SkillTab *tab;

    if (!root || !xmlStrEqual(root->name, BAD_CAST "skills"))
    {
        logger->log("Error loading skills file: %s", SKILLS_FILE);

        if (Net::getNetworkType() == ServerInfo::TMWATHENA)
        {
            SkillModel *model = new SkillModel();
            SkillInfo *skill = new SkillInfo;
            skill->id = 1;
            skill->name = "basic";
            skill->setIcon("");
            skill->modifiable = true;
            skill->visible = true;
            skill->model = model;
            skill->update();

            model->addSkill(skill);
            mSkills[1] = skill;

            model->updateVisibilities();

            listbox = new SkillListBox(model);
            scroll = new ScrollArea(listbox);
            scroll->setOpaque(false);
            scroll->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
            scroll->setVerticalScrollPolicy(ScrollArea::SHOW_ALWAYS);

            tab = new SkillTab("Skills", listbox);

            mTabs->addTab(tab, scroll);

            update();
        }
        return;
    }

    for_each_xml_child_node(set, root)
    {
        if (xmlStrEqual(set->name, BAD_CAST "set"))
        {
            setCount++;
            setName = XML::getProperty(set, "name", strprintf(_("Skill Set %d"), setCount));

            SkillModel *model = new SkillModel();

            for_each_xml_child_node(node, set)
            {
                if (xmlStrEqual(node->name, BAD_CAST "skill"))
                {
                    int id = atoi(XML::getProperty(node, "id", "-1").c_str());
                    std::string name = XML::getProperty(node, "name", strprintf(_("Skill %d"), id));
                    std::string icon = XML::getProperty(node, "icon", "");

                    SkillInfo *skill = new SkillInfo;
                    skill->id = id;
                    skill->name = name;
                    skill->setIcon(icon);
                    skill->modifiable = false;
                    skill->visible = false;
                    skill->model = model;
                    skill->update();

                    model->addSkill(skill);

                    mSkills[id] = skill;
                }
            }

            model->updateVisibilities();

            listbox = new SkillListBox(model);
            scroll = new ScrollArea(listbox);
            scroll->setOpaque(false);
            scroll->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
            scroll->setVerticalScrollPolicy(ScrollArea::SHOW_ALWAYS);

            tab = new SkillTab(setName, listbox);

            mTabs->addTab(tab, scroll);
        }
    }
    update();
}

void SkillDialog::setModifiable(int id, bool modifiable)
{
    SkillMap::iterator it = mSkills.find(id);

    if (it != mSkills.end())
    {
        SkillInfo *info = it->second;
        info->modifiable = modifiable;
        info->update();
    }
}

void SkillModel::updateVisibilities()
{
    mVisibleSkills.clear();

    for (SkillList::iterator it = mSkills.begin(); it != mSkills.end(); it++)
    {
        if ((*it)->visible)
        {
            mVisibleSkills.push_back((*it));
        }
    }
}

void SkillInfo::update()
{
    int baseLevel = PlayerInfo::getStatBase(id);
    int effLevel = PlayerInfo::getStatEffective(id);

    std::pair<int, int> exp = PlayerInfo::getStatExperience(id);

    if (!modifiable && baseLevel == 0 && effLevel == 0 && exp.second == 0)
    {
        if (visible)
        {
            visible = false;
            model->updateVisibilities();
        }

        return;
    }

    bool updateVisibility = !visible;
    visible = true;

    if (effLevel != baseLevel)
    {
        skillLevel = strprintf(_("Lvl: %d (%+d)"), baseLevel,
                               effLevel - baseLevel);
    }
    else
    {
        if (baseLevel == 0)
        {
            skillLevel.clear();
        }
        else
        {
            skillLevel = strprintf(_("Lvl: %d"), baseLevel);
        }
    }
    skillLevelWidth = -1;

    if (exp.second)
    {
        progress = exp.second != 0 ? (float) exp.first / exp.second : 0;
        skillExp = strprintf("%.2f%%", progress * 100);
    }
    else
    {
        skillExp.clear();
        progress = 0.0f;
    }

    color = Theme::getProgressColor(Theme::PROG_EXP, progress);

    if (updateVisibility)
    {
        model->updateVisibilities();
    }
}

void SkillInfo::draw(Graphics *graphics, int y, int width)
{
    graphics->drawImage(icon, 1, y);
    graphics->drawText(name, 34, y);

    if (skillLevelWidth < 0)
    {
        // Add one for padding
        skillLevelWidth = graphics->getFont()->getWidth(skillLevel) + 1;
    }

    graphics->drawText(skillLevel, width - skillLevelWidth, y);

    if (!skillExp.empty())
    {
        gcn::Rectangle rect(33, y + 15, width - 33, 17);

        ProgressBar::render(graphics, rect, color, progress, skillExp);
    }
}
