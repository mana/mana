/*
 *  The Mana World
 *  Copyright 2008 The Mana World Development Team
 *
 *  This file is part of Aethyra.
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

#ifndef _EFFECT_MANAGER_H
#define _EFFECT_MANAGER_H

#include <list>
#include <string>

class Being;

class EffectManager
{

    public: 
        struct EffectDescription
        {
             int id;
             std::string GFX;
             std::string SFX;
        };

        
        EffectManager();
        
        ~EffectManager();

        /**
         * Triggers a effect with the id, at
         * the specified being.
         */
	bool trigger(int id, Being* being);

        /**
         * Triggers a effect with the id, at
         * the specified x and y coordinate.
         */
	bool trigger(int id, int x, int y);

   private:
        std::list<EffectDescription> mEffects;

};

extern EffectManager *effectManager;

#endif // _EFFECT_MANAGER_H
