/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "gui/specialswindow.h"

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
#include "gui/widgets/flowcontainer.h"
#include "gui/widgets/windowcontainer.h"
#include "gui/setup.h"

#include "localplayer.h"
#include "log.h"

#include "net/net.h"
#include "net/specialhandler.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/xml.h"

#include <string>

#define SPECIALS_WIDTH 200
#define SPECIALS_HEIGHT 32

class SpecialEntry;

struct SpecialInfo
{
    unsigned short id;
    std::string name;
    std::string icon;
    SpecialEntry *display;
};

class SpecialEntry : public Container
{
    public:
        SpecialEntry(SpecialInfo *info);

        void update();

    protected:
        friend class SpecialsWindow;
        SpecialInfo *mInfo;

    private:
        Icon *mIcon;
        Label *mNameLabel;
        Label *mLevelLabel;
        Label *mTechLabel;
        Button *mUse;
};

SpecialsWindow::SpecialsWindow():
    Window(_("Specials"))
{
    setWindowName("Specials");
    setCloseButton(true);
    setResizable(true);
    setSaveVisible(true);
    setDefaultSize(windowContainer->getWidth() - 280, 30, 275, 425);
    setupWindow->registerWindowForReset(this);

    mTabs = new TabbedArea();

    place(0, 0, mTabs, 5, 5);

    setLocationRelativeTo(getParent());
    loadWindowState();
}

SpecialsWindow::~SpecialsWindow()
{
    // Clear gui
    loadSpecials("");
}

void SpecialsWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "use")
    {
        SpecialEntry *disp = dynamic_cast<SpecialEntry*>(event.getSource()->getParent());

        if (disp)
        {
            /*Being *target = player_node->getTarget();

            if (target)
                Net::getSpecialHandler()->use(disp->mInfo->id, 1, target->getId());
            else*/
                Net::getSpecialHandler()->use(disp->mInfo->id);
        }
    }
    else if (event.getId() == "close")
    {
        setVisible(false);
    }
}

void SpecialsWindow::logic()
{
    Window::logic();

    Tab *tab = dynamic_cast<Tab*>(mTabs->getSelectedTab());
    if (tab != mCurrentTab) {
        mCurrentTab = tab;
    }
}

std::string SpecialsWindow::update(int id)
{
    // TODO

    return std::string();
}

void SpecialsWindow::loadSpecials(const std::string &file)
{
    // TODO: mTabs->clear();
    while (mTabs->getSelectedTabIndex() != -1)
    {
        mTabs->removeTabWithIndex(mTabs->getSelectedTabIndex());
    }

    for (SpecialMap::iterator it = mSpecials.begin(); it != mSpecials.end();  it++)
    {
        delete (*it).second->display;
    }
    delete_all(mSpecials);
    mSpecials.clear();

    if (file.length() == 0)
        return;

    XML::Document doc(file);
    xmlNodePtr root = doc.rootNode();

    if (!root || !xmlStrEqual(root->name, BAD_CAST "specials"))
    {
        logger->log("Error loading specials file: %s", file.c_str());
        return;
    }

    int setCount = 0;
    std::string setName;
    ScrollArea *scroll;
    FlowContainer *container;

    for_each_xml_child_node(set, root)
    {
        if (xmlStrEqual(set->name, BAD_CAST "set"))
        {
            setCount++;
            setName = XML::getProperty(set, "name", strprintf(_("Specials Set %d"), setCount));

            container = new FlowContainer(SPECIALS_WIDTH, SPECIALS_HEIGHT);
            container->setOpaque(false);
            scroll = new ScrollArea(container);
            scroll->setOpaque(false);
            scroll->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
            scroll->setVerticalScrollPolicy(ScrollArea::SHOW_ALWAYS);

            mTabs->addTab(setName, scroll);
            for_each_xml_child_node(node, set)
            {
                if (xmlStrEqual(node->name, BAD_CAST "special"))
                {
                    int id = atoi(XML::getProperty(node, "id", "-1").c_str());
                    if (id == -1)
                        continue;
                    std::string name = XML::getProperty(node, "name", strprintf(_("Special %d"), id));
                    std::string icon = XML::getProperty(node, "icon", "");

                    SpecialInfo *special = new SpecialInfo;
                    special->id = id;
                    special->name = name;
                    special->icon = icon;
                    special->display = new SpecialEntry(special);

                    container->add(special->display);

                    mSpecials[id] = special;
                }
            }
        }
    }
}

SpecialEntry::SpecialEntry(SpecialInfo *info) :
    mInfo(info),
    mIcon(NULL),
    mNameLabel(new Label(info->name)),
    mLevelLabel(new Label("999")),
    mUse(new Button("Use", "use", specialsWindow))
{
    setFrameSize(1);
    setOpaque(false);
    setSize(SPECIALS_WIDTH, SPECIALS_HEIGHT);

    if (!info->icon.empty())
        mIcon = new Icon(info->icon);
    else
        mIcon = new Icon("graphics/gui/unknown-item.png");

    mIcon->setPosition(1, 0);
    add(mIcon);

    mNameLabel->setPosition(35, 0);
    add(mNameLabel);

    mLevelLabel->setPosition(getWidth() - mLevelLabel->getWidth(), 0);
    add(mLevelLabel);

    mNameLabel->setWidth(mLevelLabel->getX() - mNameLabel->getX() - 1);

    mUse->setPosition(getWidth() - mUse->getWidth(), 13);
    add(mUse);

    update();
}

void SpecialEntry::update()
{
    // TODO
}
