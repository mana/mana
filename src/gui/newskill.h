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

#include <guichan.hpp>
#include "window.h"
 
#define N_SKILL 100 // skill count constant
 
struct nSkill {
    short level;
    short exp;
};
 
class NewSkillDialog : public Window, public gcn::ActionListener
{
    private:
        // members
        
        gcn::ScrollArea *skillScrollArea;
        gcn::Container *skillList;
        gcn::Label *skillLabel;
        gcn::Button *groupButtons;
        gcn::Button *closeButton;
                    
    public:
        /**
         * Constructor.
         */
        NewSkillDialog();

        /**
         * Destructor.
         */
        ~NewSkillDialog();  
        
        // action listener
        void action(const std::string&);     
};

#endif
