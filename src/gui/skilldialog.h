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

#ifndef SKILLDIALOG_H
#define SKILLDIALOG_H

#include "gui/widgets/window.h"
#include "eventlistener.h"

#include <guichan/actionlistener.hpp>

#include <map>

class Button;
class Label;
class ScrollArea;
class Tab;
class TabbedArea;

struct SkillInfo;

/**
 * The skill dialog.
 *
 * \ingroup Interface
 */
class SkillDialog : public Window, public gcn::ActionListener, public EventListener
{
    public:
        SkillDialog();

        ~SkillDialog();

        void event(Event::Channel channel, const Event &event);

        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event);

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

        bool hasSkills() { return !mSkills.empty(); }

    private:
        typedef std::map<int, SkillInfo*> SkillMap;
        SkillMap mSkills;
        TabbedArea *mTabs;
};

extern SkillDialog *skillDialog;

#endif
