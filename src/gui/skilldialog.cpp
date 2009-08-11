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
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/tab.h"
#include "gui/widgets/tabbedarea.h"
#include "gui/widgets/vertcontainer.h"
#include "gui/widgets/windowcontainer.h"
#include "gui/setup.h"

#include "localplayer.h"
#include "log.h"

#include "net/net.h"
#include "net/playerhandler.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/xml.h"

#include <string>

class SkillEntry;

struct SkillInfo
{
    unsigned short id;
    std::string name;
    std::string icon;
    bool modifiable;
    SkillEntry *display;
};

class SkillEntry : public Container, gcn::WidgetListener
{
    public:
        SkillEntry(SkillInfo *info);

        void widgetResized(const gcn::Event &event);

        void update();

    protected:
        friend class SkillDialog;
        SkillInfo *mInfo;

    private:
        Icon *mIcon;
        Label *mNameLabel;
        Label *mLevelLabel;
        Label *mExpLabel;
        Button *mIncrease;
        ProgressBar *mProgress;
};

SkillDialog::SkillDialog():
    Window(_("Skills"))
{
    setWindowName("Skills");
    setCloseButton(true);
    setResizable(true);
    setSaveVisible(true);
    setDefaultSize(windowContainer->getWidth() - 280, 30, 275, 425);
    setupWindow->registerWindowForReset(this);

    mTabs = new TabbedArea();
    mPointsLabel = new Label("0");

    place(0, 0, mTabs, 5, 5);
    place(0, 5, mPointsLabel);

    setLocationRelativeTo(getParent());
    loadWindowState();
}

SkillDialog::~SkillDialog()
{
    // Clear gui
    loadSkills("");
}

void SkillDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "inc")
    {
        SkillEntry *disp = dynamic_cast<SkillEntry*>(event.getSource()->getParent());

        if (disp)
            Net::getPlayerHandler()->increaseSkill(disp->mInfo->id);
    }
    else if (event.getId() == "close")
    {
        setVisible(false);
    }
}

void SkillDialog::adjustTabSize()
{
    gcn::Widget *content = mTabs->getCurrentWidget();
    if (content) {
        int width = mTabs->getWidth() - 2 * content->getFrameSize() - 2 * mTabs->getFrameSize();
        int height = mTabs->getContainerHeight() - 2 * content->getFrameSize();
        content->setSize(width, height);
        content->setVisible(true);
        content->logic();
    }
}

void SkillDialog::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);

    adjustTabSize();
}

void SkillDialog::logic()
{
    Window::logic();

    Tab *tab = dynamic_cast<Tab*>(mTabs->getSelectedTab());
    if (tab != mCurrentTab) {
        mCurrentTab = tab;
        adjustTabSize();
    }
}

std::string SkillDialog::update(int id)
{
    SkillMap::iterator i = mSkills.find(id);

    if (i != mSkills.end())
    {
        SkillInfo *info = i->second;
        info->display->update();
        return info->name;
    }

    return std::string();
}

void SkillDialog::update()
{
    mPointsLabel->setCaption(strprintf(_("Skill points available: %d"),
                                       player_node->getSkillPoints()));
    mPointsLabel->adjustSize();

    for (SkillMap::iterator it = mSkills.begin(); it != mSkills.end(); it++)
    {
        if ((*it).second->modifiable)
            (*it).second->display->update();
    }
}

void SkillDialog::loadSkills(const std::string &file)
{
    // TODO: mTabs->clear();
    while (mTabs->getSelectedTabIndex() != -1)
    {
        mTabs->removeTabWithIndex(mTabs->getSelectedTabIndex());
    }

    for (SkillMap::iterator it = mSkills.begin(); it != mSkills.end();  it++)
    {
        delete (*it).second->display;
    }
    delete_all(mSkills);
    mSkills.clear();

    if (file.length() == 0)
        return;

    XML::Document doc(file);
    xmlNodePtr root = doc.rootNode();

    if (!root || !xmlStrEqual(root->name, BAD_CAST "skills"))
    {
        logger->log("Error loading skills file: %s", file.c_str());
        return;
    }

    int setCount = 0;
    std::string setName;
    ScrollArea *scroll;
    VertContainer *container;

    for_each_xml_child_node(set, root)
    {
        if (xmlStrEqual(set->name, BAD_CAST "set"))
        {
            setCount++;
            setName = XML::getProperty(set, "name", strprintf(_("Skill Set %d"), setCount));

            container = new VertContainer(32);
            container->setOpaque(false);
            scroll = new ScrollArea(container);
            scroll->setOpaque(false);
            scroll->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
            scroll->setVerticalScrollPolicy(ScrollArea::SHOW_ALWAYS);

            mTabs->addTab(setName, scroll);
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
                    skill->icon = icon;
                    skill->modifiable = 0;
                    skill->display = new SkillEntry(skill);

                    container->add(skill->display);

                    mSkills[id] = skill;
                }
            }
        }
    }

    adjustTabSize();
    update();
}

void SkillDialog::setModifiable(int id, bool modifiable)
{
    SkillMap::iterator i = mSkills.find(id);

    if (i != mSkills.end())
    {
        SkillInfo *info = i->second;
        info->modifiable = modifiable;
        info->display->update();
    }
}

SkillEntry::SkillEntry(SkillInfo *info) :
    mInfo(info),
    mIcon(NULL),
    mNameLabel(new Label(info->name)),
    mLevelLabel(new Label("999")),
    mIncrease(new Button(_("+"), "inc", skillDialog)),
    mProgress(new ProgressBar(0.0f, 200, 20, gcn::Color(150, 150, 150)))
{
    setFrameSize(1);
    setOpaque(false);

    addWidgetListener(this);

    if (!info->icon.empty())
        mIcon = new Icon(info->icon);
    else
        mIcon = new Icon("graphics/gui/unknown-item.png");

    mIcon->setPosition(1, 0);
    add(mIcon);

    mNameLabel->setPosition(35, 0);
    add(mNameLabel);

    mLevelLabel->setPosition(165, 0);
    add(mLevelLabel);

    mProgress->setPosition(35, 13);
    add(mProgress);

    mIncrease->setPosition(getWidth() - mIncrease->getWidth(), 13);
    add(mIncrease);

    update();
}

void SkillEntry::widgetResized(const gcn::Event &event)
{
    gcn::Rectangle size = getChildrenArea();

    if (mProgress->isVisible() && mIncrease->isVisible())
    {
        mLevelLabel->setPosition(size.width - mLevelLabel->getWidth()
                                 - mIncrease->getWidth() - 4, 0);
        mProgress->setWidth(size.width - mIncrease->getWidth() - 39);
        mIncrease->setPosition(getWidth() - mIncrease->getWidth() - 2, 6);
    }
    else if (mProgress->isVisible())
    {
        mLevelLabel->setPosition(size.width - mLevelLabel->getWidth(), 0);
        mProgress->setWidth(size.width - 39);
    }
    else if (mIncrease->isVisible())
    {
        mLevelLabel->setPosition(size.width - mLevelLabel->getWidth()
                                 - mIncrease->getWidth() - 4, 0);
        mIncrease->setPosition(getWidth() - mIncrease->getWidth() - 2, 6);
    }
    else
        mLevelLabel->setPosition(size.width - mLevelLabel->getWidth(), 0);
}

void SkillEntry::update()
{
    int baseLevel = player_node->getAttributeBase(mInfo->id);
    int effLevel = player_node->getAttributeEffective(mInfo->id);

    if (baseLevel <= 0 && !mInfo->modifiable)
    {
        setVisible(false);
        return;
    }

    setVisible(true);

    std::string skillLevel;

    if (effLevel != baseLevel)
    {
        skillLevel = strprintf(_("Lvl: %d (%+d)"),
                               baseLevel, baseLevel - effLevel);
    }
    else
    {
        skillLevel = strprintf(_("Lvl: %d"), baseLevel);
    }

    mLevelLabel->setCaption(skillLevel);

    std::pair<int, int> exp = player_node->getExperience(mInfo->id);
    std::string sExp (toString(exp.first) + " / " + toString(exp.second));

    mLevelLabel->adjustSize();

    if (exp.second)
    {
        mProgress->setVisible(true);
        mProgress->setText(sExp);

        // More intense red as exp grows
        int color = 150 - (int)(150 * ((float) exp.first / exp.second));
        mProgress->setColor(244, color, color);
        mProgress->setProgress((float) exp.first / exp.second);
    }
    else
        mProgress->setVisible(false);

    if (mInfo->modifiable)
    {
        mIncrease->setVisible(true);
        mIncrease->setEnabled(player_node->getSkillPoints());
    }
    else
    {
        mIncrease->setVisible(false);
        mIncrease->setEnabled(false);
    }

    widgetResized(NULL);
}
