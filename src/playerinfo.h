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

#ifndef _TMW_PLAYERINFO_H
#define _TMW_PLAYERINFO_H

#include <vector>
#include "gui/skill.h"

struct PLAYER_INFO {
    int id;
    float lastAttackTime; // used to synchronize the charge dialog
    char name[24];
    short hp, max_hp, sp, max_sp, lv;
    short statsPointsToAttribute;
    int xp, xpForNextLevel, gp, job_xp, jobXpForNextLevel, job_lv;
    short statp, skill_point, hair_color, hair_style;
    char STR, AGI, VIT, INT, DEX, LUK;
    char STRUp, AGIUp, VITUp, INTUp, DEXUp, LUKUp;
    int totalWeight, maxWeight;
    short weapon;
    // skill list declaration
    std::vector<SKILL> m_Skill; // array of N_SKILLS skills
    // gets the requested skills level from char_info
    int GetSkill(int n_ID, int n_XP=2, int n_base = false); // implemented in the body (main.cpp)
};

extern PLAYER_INFO *char_info;

#endif
