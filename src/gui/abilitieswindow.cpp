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

#include "gui/abilitieswindow.h"

#include "log.h"

#include "gui/setup.h"

#include "gui/widgets/button.h"
#include "gui/widgets/container.h"
#include "gui/widgets/icon.h"
#include "gui/widgets/label.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/windowcontainer.h"

#include "net/net.h"
#include "net/abilityhandler.h"

#include "resources/abilitydb.h"
#include "resources/theme.h"

#include "utils/dtor.h"
#include "utils/gettext.h"

#include "localplayer.h"

#include <string>

#define ABILITIES_WIDTH 200
#define ABILITIES_HEIGHT 32

class AbilityEntry : public Container
{
    public:
        AbilityEntry(AbilityInfo *info);

        void update(int current, int needed);

    protected:
        friend class AbilitiesWindow;
        AbilityInfo *mInfo;

    private:
        Icon *mIcon = nullptr;          // icon to display
        Label *mNameLabel = nullptr;    // name to display
        Button *mUse = nullptr;         // use button (only shown when applicable)
        ProgressBar *mRechargeBar = nullptr; // recharge bar (only shown when applicable)
};

AbilitiesWindow::AbilitiesWindow():
    Window(_("Abilities"))
{
    setWindowName("Abilities");
    setCloseButton(true);
    setResizable(true);
    setSaveVisible(true);
    setDefaultSize(windowContainer->getWidth() - 280, 40, ABILITIES_WIDTH + 20, 225);
    setupWindow->registerWindowForReset(this);

    center();
    loadWindowState();
}

AbilitiesWindow::~AbilitiesWindow()
{
    // Clear gui
}

void AbilitiesWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "use")
    {
        auto *disp = dynamic_cast<AbilityEntry*>(event.getSource()->getParent());

        if (disp)
        {
            if (disp->mInfo->targetMode == AbilityInfo::TARGET_BEING)
            {
                Being *target = local_player->getTarget();

                if (target)
                    Net::getAbilityHandler()->use(disp->mInfo->id, 1, target->getId());
                else
                    Net::getAbilityHandler()->use(disp->mInfo->id);
            }
            else
            {
                // TODO: Allow the player to aim at a position on the map and
                //       Use abilities on the map position.
            }
        }
    }
    else if (event.getId() == "close")
    {
        setVisible(false);
    }
}

void AbilitiesWindow::draw(gcn::Graphics *graphics)
{
    // update the progress bars
    const std::map<int, Ability> &abilityData = PlayerInfo::getAbilityStatus();
    bool foundNew = false;
    unsigned int found = 0; // number of entries in abilityData which match mEntries

    for (auto &[id, ability] : abilityData)
    {
        auto e = mEntries.find(id);
        if (e == mEntries.end())
        {
            // found a new ability - abort update and rebuild from scratch
            foundNew = true;
            break;
        }

        // update progress bar of ability
        e->second->update(ability.currentMana, ability.neededMana);
        found++;
    }
    // a rebuild is needed when a) the number of abilities changed or b) an existing entry isn't found anymore
    if (foundNew || found != mEntries.size())
        rebuild(abilityData);

    Window::draw(graphics);
}

void AbilitiesWindow::rebuild(const std::map<int, Ability> &abilityData)
{
    delete_all(mEntries);
    
    mEntries.clear();
    int vPos = 0; //vertical position of next placed element

    for (auto &[id, ability] : abilityData)
    {
        logger->log("Updating ability GUI for %d", id);

        AbilityInfo *info = AbilityDB::get(id);
        if (info)
        {
            info->rechargeCurrent = ability.currentMana;
            info->rechargeNeeded = ability.neededMana;
            auto *entry = new AbilityEntry(info);
            entry->setPosition(0, vPos);
            vPos += entry->getHeight() + 3;
            add(entry);
            mEntries[id] = entry;
        }
        else
        {
            logger->log("Warning: No info available of ability %d", id);
        }
    }
}


AbilityEntry::AbilityEntry(AbilityInfo *info) :
    mInfo(info)
{
    setSize(ABILITIES_WIDTH, ABILITIES_HEIGHT);

    if (!info->icon.empty())
        mIcon = new Icon(info->icon);
    else
        mIcon = new Icon(Theme::resolveThemePath("unknown-item.png"));

    mIcon->setPosition(1, 0);
    add(mIcon);

    mNameLabel = new Label(info->name);
    mNameLabel->setPosition(35, 0);
    add(mNameLabel);

    mUse = new Button("Use", "use", abilitiesWindow);
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

void AbilityEntry::update(int current, int needed)
{
    if (mRechargeBar)
    {
        float progress = (float)current / (float)needed;
        mRechargeBar->setProgress(progress);
    }
}
