/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "gui/widgets/window.h"
#include "eventlistener.h"

#include <guichan/actionlistener.hpp>
#include <guichan/selectionlistener.hpp>

#include <map>
#include <memory>
#include <vector>

class Button;
class Label;
class ScrollArea;
class SkillModel;
class Tab;
class TabbedArea;

struct SkillInfo;

/**
 * The skill dialog.
 *
 * \ingroup Interface
 */
class SkillDialog : public Window,
                    public gcn::ActionListener,
                    public gcn::SelectionListener,
                    public EventListener
{
    public:
        SkillDialog();
        ~SkillDialog() override;

        void event(Event::Channel channel, const Event &event) override;

        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event) override;

        /**
         * Called when the selected skill or tab changes.
         */
        void valueChanged(const gcn::SelectionEvent &event) override;

        /**
         * Update the given skill's display
         */
        std::string update(int id);

        /**
         * Update other parts of the display
         */
        void update();

        void loadSkills();

        void clearSkills();

        void setModifiable(int id, bool modifiable);

        /**
         * Updates the enabled state and tooltip of the "Up" button based on
         * the currently selected skill and the available skill points.
         */
        void updateIncreaseButton();

        bool hasSkills() const { return !mSkills.empty(); }

    private:
        std::vector<std::unique_ptr<SkillModel>> mSkillModels;
        std::vector<std::unique_ptr<Tab>> mTabs;
        std::vector<std::unique_ptr<gcn::Widget>> mTabWidgets;
        std::map<int, SkillInfo> mSkills;
        TabbedArea *mTabbedArea;
        Label *mPointsLabel;
        Button *mIncreaseButton;
};

extern SkillDialog *skillDialog;
