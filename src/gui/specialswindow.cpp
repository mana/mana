/*
 *  The Mana Client
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

#include "gui/specialswindow.h"

#include "log.h"

#include "gui/setup.h"

#include "gui/widgets/button.h"
#include "gui/widgets/container.h"
#include "gui/widgets/icon.h"
#include "gui/widgets/label.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/windowcontainer.h"

#include "net/net.h"
#include "net/specialhandler.h"

#include "resources/specialdb.h"
#include "resources/theme.h"

#include "utils/dtor.h"
#include "utils/gettext.h"

#include "localplayer.h"

#include <string>

#define SPECIALS_WIDTH 200
#define SPECIALS_HEIGHT 32

class SpecialEntry : public Container
{
    public:
        SpecialEntry(SpecialInfo *info);

        void update(int current, int needed);

    protected:
        friend class SpecialsWindow;
        SpecialInfo *mInfo;

    private:
        Icon *mIcon = nullptr;          // icon to display
        Label *mNameLabel = nullptr;    // name to display
        Button *mUse = nullptr;         // use button (only shown when applicable)
        ProgressBar *mRechargeBar = nullptr; // recharge bar (only shown when applicable)
};

SpecialsWindow::SpecialsWindow():
    Window(_("Specials"))
{
    setWindowName("Specials");
    setCloseButton(true);
    setResizable(true);
    setSaveVisible(true);
    setDefaultSize(windowContainer->getWidth() - 280, 40, SPECIALS_WIDTH + 20, 225);
    setupWindow->registerWindowForReset(this);

    center();
    loadWindowState();
}

SpecialsWindow::~SpecialsWindow()
{
    // Clear gui
}

void SpecialsWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "use")
    {
        auto *disp = dynamic_cast<SpecialEntry*>(event.getSource()->getParent());

        if (disp)
        {
            if (disp->mInfo->targetMode == SpecialInfo::TARGET_BEING)
            {
                Being *target = local_player->getTarget();

                if (target)
                    Net::getSpecialHandler()->use(disp->mInfo->id, 1, target->getId());
                else
                    Net::getSpecialHandler()->use(disp->mInfo->id);
            }
            else
            {
                // TODO: Allow the player to aim at a position on the map and
                //       Use special on the map position.
            }
        }
    }
    else if (event.getId() == "close")
    {
        setVisible(false);
    }
}

void SpecialsWindow::draw(gcn::Graphics *graphics)
{
    // update the progress bars
    std::map<int, Special> specialData = PlayerInfo::getSpecialStatus();
    bool foundNew = false;
    unsigned int found = 0; // number of entries in specialData which match mEntries

    for (auto &special : specialData)
    {
        auto e = mEntries.find(special.first);
        if (e == mEntries.end())
        {
            // found a new special - abort update and rebuild from scratch
            foundNew = true;
            break;
        }

        // update progress bar of special
        e->second->update(special.second.currentMana, special.second.neededMana);
        found++;
    }
    // a rebuild is needed when a) the number of specials changed or b) an existing entry isn't found anymore
    if (foundNew || found != mEntries.size())
        rebuild(specialData);

    Window::draw(graphics);
}

void SpecialsWindow::rebuild(const std::map<int, Special> &specialData)
{
    delete_all(mEntries);
    
    mEntries.clear();
    int vPos = 0; //vertical position of next placed element

    for (auto special : specialData)
    {
        logger->log("Updating special GUI for %d", special.first);

        SpecialInfo *info = SpecialDB::get(special.first);
        if (info)
        {
            info->rechargeCurrent = special.second.currentMana;
            info->rechargeNeeded = special.second.neededMana;
            auto* entry = new SpecialEntry(info);
            entry->setPosition(0, vPos);
            vPos += entry->getHeight() + 3;
            add(entry);
            mEntries[special.first] = entry;
        }
        else
        {
            logger->log("Warning: No info available of special %d", special.first);
        }
    }
}


SpecialEntry::SpecialEntry(SpecialInfo *info) :
    mInfo(info)
{
    setSize(SPECIALS_WIDTH, SPECIALS_HEIGHT);

    if (!info->icon.empty())
        mIcon = new Icon(info->icon);
    else
        mIcon = new Icon(Theme::resolveThemePath("unknown-item.png"));

    mIcon->setPosition(1, 0);
    add(mIcon);


    mNameLabel = new Label(info->name);
    mNameLabel->setPosition(35, 0);
    add(mNameLabel);

    mUse = new Button("Use", "use", specialsWindow);
    mUse->setPosition(getWidth() - mUse->getWidth(), 5);
    add(mUse);

    if (info->rechargeable)
    {
        float progress = (float)info->rechargeCurrent / (float)info->rechargeNeeded;
        mRechargeBar = new ProgressBar(progress, 100, 10, Theme::PROG_MP);
        mRechargeBar->setSmoothProgress(false);
        mRechargeBar->setPosition(mNameLabel->getX(), 18);
        add(mRechargeBar);
    }
}

void SpecialEntry::update(int current, int needed)
{
    if (mRechargeBar)
    {
        float progress = (float)current / (float)needed;
        mRechargeBar->setProgress(progress);
    }
}
