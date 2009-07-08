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

#ifndef SKILL_H
#define SKILL_H

#include "gui/widgets/window.h"

#include "guichanfwd.h"

#include <guichan/actionlistener.hpp>

#include <list>
#include <map>

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
class SkillDialog : public Window, public gcn::ActionListener
{
    public:
        SkillDialog();

        ~SkillDialog();

        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Called when the widget changes size. Used for adapting the size of
         * the tabbed area.
         */
        void widgetResized(const gcn::Event &event);

        void logic();

        /**
         * Update the given skill's display
         */
        std::string update(int id);

        /**
         * Update other parts of the display
         */
        void update();

        void loadSkills(const std::string &file, bool fixed = true);

        void setModifiable(int id, bool modifiable);

    private:
        void adjustTabSize();

        typedef std::map<int, SkillInfo*> SkillMap;
        SkillMap mSkills;
        Tab *mCurrentTab;
        TabbedArea *mTabs;
        Label *mPointsLabel;
};

extern SkillDialog *skillDialog;

#endif
