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

#include <guichan/actionlistener.hpp>

#include <vector>

struct SKILL
{
    short id;      /**< Index into "skill_db" array */
    short lv, sp;
};

class GuiTable;
class ScrollArea;
class SkillGuiTableModel;

/**
 * The skill dialog.
 *
 * \ingroup Interface
 */
class SkillDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        SkillDialog();

        /**
         * Destructor.
         */
        ~SkillDialog();

        void action(const gcn::ActionEvent &event);

        void update();

        int getNumberOfElements();

        bool hasSkill(int id);
        void addSkill(int id, int lv, int sp);
        void setSkill(int id, int lv, int sp);
        void cleanList();

        const std::vector<SKILL*>& getSkills() const { return mSkillList; }

    private:
        GuiTable *mTable;
        ScrollArea *skillScrollArea;
        SkillGuiTableModel *mTableModel;
        gcn::Label *mPointsLabel;
        gcn::Button *mIncButton;
        gcn::Button *mUseButton;

        std::vector<SKILL*> mSkillList;
};

extern SkillDialog *skillDialog;

#endif
