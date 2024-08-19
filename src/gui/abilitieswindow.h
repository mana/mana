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

#pragma once

#include "playerinfo.h"

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

#include <map>

class AbilityEntry;

class AbilitiesWindow : public Window, public gcn::ActionListener
{
    public:
        AbilitiesWindow();

        ~AbilitiesWindow() override;

        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &actionEvent) override;

        void draw(gcn::Graphics *graphics) override;

        bool hasAbilities() const
        { return !mEntries.empty(); }

    private:
        // (re)constructs the list of abilities
        void rebuild(const std::map<int, Ability> &abilityData);

        std::map<int, AbilityEntry *> mEntries;
};

extern AbilitiesWindow *abilitiesWindow;

