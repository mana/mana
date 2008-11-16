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
 */

#ifndef _EFFECT_MANAGER_H
#define _EFFECT_MANAGER_H

#include <string>
#include <list>

class EffectManager
{

    public: 
        struct EffectDescription {
             int id;
             std::string GFX;
             std::string SFX;
        };

        
        EffectManager();
        
        ~EffectManager();

        /**
         * Triggers a effect with the id, at x,y
         * returns true if ID exists
         */
	bool trigger(int id, int x = 0, int y = 0);

   private:
        std::list<EffectDescription> mEffects;

};

extern EffectManager *effectManager;

#endif // _EFFECT_MANAGER_H
