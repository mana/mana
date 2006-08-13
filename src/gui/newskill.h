/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#ifndef _TMW_NSKILL_H
#define _TMW_NSKILL_H

#include <guichan/actionlistener.hpp>

#include "window.h"

#include "../guichanfwd.h"

class ProgressBar;

#define N_SKILL 100 // skill count constant
#define N_SKILL_CAT 9 // skill category count
#define N_SKILL_CAT_SIZE 10 // skill category maximum size

struct nSkill {
    short level;
    short exp;
};

/**
 * Dialog showing the skills in the planned skill model.
 *
 * \ingroup Interface
 */
class NewSkillDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        NewSkillDialog();

        // action listener
        void action(const std::string& eventId, gcn::Widget* widget);

    private:
        void resetNSD(); // updates the values in the dialog box

        // members
        int startPoint; // starting point of skill listing
        ProgressBar *mSkillbar[N_SKILL_CAT_SIZE];
        gcn::Label *mSkillLabel[N_SKILL_CAT_SIZE];
        gcn::Label *mSkillLevel[N_SKILL_CAT_SIZE];
        nSkill mPlayerSkill[N_SKILL]; // pointer to an array of skill values
};

#endif
