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

#ifndef _TMW_SKILL_H
#define _TMW_SKILL_H


#include <guichan.hpp>
#include "window.h"
#include <vector>
/**
 * The skill dialog.
 *
 * \ingroup GUI
 */
 
// skill struct, by Kyokai
// skill names are stored in a table elsewhere. (SkillDialog.cpp)
typedef struct {
       short level; // level of the skill
       short exp; // exp value
       double mod; // value of the modifier (0,0.5,1,2,4)
       // next value can be calculated when needed by the function:
       // 20 * level^1.2
       } SKILL;
 
class SkillDialog : public Window, public gcn::ActionListener
//                    public gcn::ListModel
{
    private:
        gcn::ListBox *skillListBox;
        gcn::ScrollArea *skillScrollArea;
        gcn::Label *pointsLabel;
        gcn::Button *incButton;
        gcn::Button *closeButton;

    public:
        /**
         * Constructor.
         */
        SkillDialog();

        /**
         * Destructor.
         */
        ~SkillDialog();

        void action(const std::string&);
};

#endif
